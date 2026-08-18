const { execFileSync } = require('child_process');
const fs = require('fs');
const path = require('path');

const CLANG_TIDY_BIN = 'clang-tidy';
const CLANG_FORMAT_BIN = 'clang-format';
const CHECK_CONFIG = '{CheckOptions: {readability-identifier-naming.FunctionCase: lower_case}}';

// clang-tidy's plain-text diagnostic shape for this check (no --export-fixes/YAML
// dependency needed, matching remove-duplicate-struct-decls's preference for
// avoiding a bundled YAML parser):
//
//   path/to/file.h:31:6: warning: invalid case style for function 'fooBar' [readability-identifier-naming]
//      31 | void fooBar(int x);
//         |      ^~~~~~
//         |      foo_bar
const WARNING_RE = /^(.+?):(\d+):(\d+): warning: invalid case style for function '([^']+)' \[readability-identifier-naming\]$/;
const CARET_LINE_RE = /^\s*\|\s*\^/;
const SUGGESTION_LINE_RE = /^\s*\|\s*(\S+)\s*$/;
const FIXIT_RE = /^(.+?):(\d+):(\d+): note: FIX-IT applied suggested code changes$/;
const APPLIED_RE = /clang-tidy applied (\d+) of (\d+) suggested fixes\.?/;

function realPath(p) {
  try {
    return fs.realpathSync(p);
  } catch {
    return p;
  }
}

function loadCompileDbFiles(compileDbPath) {
  const entries = JSON.parse(fs.readFileSync(compileDbPath, 'utf8'));
  const seen = new Set();
  const files = [];
  for (const entry of entries) {
    if (!entry.file) continue;
    const rp = realPath(entry.file);
    if (seen.has(rp) || !fs.existsSync(rp)) continue;
    seen.add(rp);
    files.push(rp);
  }
  return files.sort();
}

function runClangTidy(filePath, compileDbDir, extraArgs) {
  const args = [
    `-p=${compileDbDir}`,
    '--checks=-*,readability-identifier-naming',
    `--config=${CHECK_CONFIG}`,
    ...extraArgs,
    filePath
  ];
  try {
    return execFileSync(CLANG_TIDY_BIN, args, {
      encoding: 'utf8',
      maxBuffer: 64 * 1024 * 1024,
      stdio: ['ignore', 'pipe', 'pipe']
    });
  } catch (err) {
    if (typeof err.stdout === 'string') return err.stdout;
    throw new Error(err.stderr || err.message);
  }
}

// Replays the same "diagnostic, source line, caret line, suggestion line" shape
// clang-tidy prints per finding to recover (oldName -> newName) without --export-fixes.
function parseDryRun(stdout) {
  const lines = stdout.split(/\r?\n/);
  const renames = [];
  for (let i = 0; i < lines.length; i++) {
    const m = WARNING_RE.exec(lines[i]);
    if (!m) continue;
    const [, file, line, , oldName] = m;
    let newName = null;
    for (let j = i + 1; j < Math.min(i + 6, lines.length); j++) {
      const l = lines[j];
      if (WARNING_RE.test(l)) break;
      if (CARET_LINE_RE.test(l)) continue;
      const sm = SUGGESTION_LINE_RE.exec(l);
      if (sm) {
        newName = sm[1];
        break;
      }
    }
    if (newName) renames.push({ file, line: Number(line), oldName, newName });
  }
  return renames;
}

function parseTouchedFiles(stdout) {
  const files = new Set();
  for (const line of stdout.split(/\r?\n/)) {
    const m = FIXIT_RE.exec(line);
    if (m) files.add(m[1]);
  }
  return files;
}

function parseAppliedCount(stdout) {
  const m = APPLIED_RE.exec(stdout);
  return m ? Number(m[1]) : 0;
}

module.exports = async function ({ vscode, workspaceRoot, output }) {
  try {
    execFileSync(CLANG_TIDY_BIN, ['--version'], { stdio: 'ignore' });
  } catch {
    vscode.window.showErrorMessage(`Enforce Snake Case Function Names: "${CLANG_TIDY_BIN}" was not found on PATH.`);
    return;
  }

  const compileDbDir = path.join(workspaceRoot, 'build');
  const compileDbPath = path.join(compileDbDir, 'compile_commands.json');
  if (!fs.existsSync(compileDbPath)) {
    vscode.window.showErrorMessage(
      `Enforce Snake Case Function Names: no compile_commands.json found at ${compileDbPath}. ` +
      'Configure the CMake build with CMAKE_EXPORT_COMPILE_COMMANDS=ON first.'
    );
    return;
  }

  const scopePick = await vscode.window.showQuickPick(
    [
      {
        label: 'Entire workspace',
        detail: 'Renames declarations, definitions, and call sites across every file in the compile database. Recommended.'
      },
      {
        label: 'Current file only',
        detail: "Faster, but a function used from other translation units is left inconsistent (won't compile) until those files are also processed."
      }
    ],
    { placeHolder: 'Rename functions to snake_case across which scope?' }
  );
  if (!scopePick) return;

  let targetFiles;
  if (scopePick.label === 'Current file only') {
    const filePath = vscode.window.activeTextEditor?.document?.uri?.fsPath;
    if (!filePath) {
      vscode.window.showWarningMessage('Enforce Snake Case Function Names: no active editor.');
      return;
    }
    const rp = realPath(filePath);
    if (!loadCompileDbFiles(compileDbPath).includes(rp)) {
      vscode.window.showWarningMessage('Enforce Snake Case Function Names: the active file is not part of the compile database.');
      return;
    }
    targetFiles = [rp];
  } else {
    targetFiles = loadCompileDbFiles(compileDbPath);
  }

  output.show();
  output.appendLine(`Scanning ${targetFiles.length} file(s) for non-snake_case function names (dry run, nothing written yet)...`);

  const byOldName = new Map(); // oldName -> newName
  const byNewName = new Map(); // newName -> Set(oldName)
  let dryRunErrors = 0;

  for (const [i, filePath] of targetFiles.entries()) {
    output.appendLine(`  [${i + 1}/${targetFiles.length}] checking ${filePath}`);
    let stdout;
    try {
      stdout = runClangTidy(filePath, compileDbDir, []);
    } catch (err) {
      dryRunErrors++;
      output.appendLine(`    [error] ${err.message}`);
      continue;
    }
    for (const r of parseDryRun(stdout)) {
      byOldName.set(r.oldName, r.newName);
      if (!byNewName.has(r.newName)) byNewName.set(r.newName, new Set());
      byNewName.get(r.newName).add(r.oldName);
    }
  }

  const renameCount = byOldName.size;
  if (renameCount === 0) {
    const suffix = dryRunErrors > 0 ? ` (${dryRunErrors} file(s) failed to parse — see output)` : '';
    vscode.window.showInformationMessage(`Enforce Snake Case Function Names: no non-snake_case function names found${suffix}.`);
    return;
  }

  const collisions = [...byNewName.entries()].filter(([, olds]) => olds.size > 1);
  output.appendLine(`\nFound ${renameCount} function name(s) needing a snake_case rename.`);

  if (collisions.length > 0) {
    output.appendLine(`${collisions.length} target name(s) collide (two or more distinct functions would rename to the same identifier):`);
    for (const [newName, olds] of collisions) {
      output.appendLine(`  '${newName}' <- ${[...olds].join(', ')}`);
    }
    output.appendLine('Resolve these collisions manually (e.g. rename one of the colliding functions) before re-running. No files were modified.');
    vscode.window.showWarningMessage(
      `Enforce Snake Case Function Names: found ${collisions.length} naming collision(s) — see output for details. Nothing was changed.`
    );
    return;
  }

  const proceed = await vscode.window.showWarningMessage(
    `This will rewrite ${renameCount} function name(s) to snake_case across ${targetFiles.length} file(s) using clang-tidy --fix, then reformat touched files with clang-format. ` +
    'Make sure your work is committed or stashed so you can review the diff. Continue?',
    { modal: true },
    'Apply'
  );
  if (proceed !== 'Apply') return;

  output.appendLine('\nApplying fixes...');
  const touchedFiles = new Set();
  let applyErrors = 0;
  let totalApplied = 0;

  for (const [i, filePath] of targetFiles.entries()) {
    output.appendLine(`  [${i + 1}/${targetFiles.length}] fixing ${filePath}`);
    let stdout;
    try {
      stdout = runClangTidy(filePath, compileDbDir, ['--fix', '--fix-errors']);
    } catch (err) {
      applyErrors++;
      output.appendLine(`    [error] ${err.message}`);
      continue;
    }
    for (const f of parseTouchedFiles(stdout)) touchedFiles.add(f);
    totalApplied += parseAppliedCount(stdout);
  }

  let formatFailed = false;
  if (touchedFiles.size > 0) {
    try {
      execFileSync(CLANG_FORMAT_BIN, ['--version'], { stdio: 'ignore' });
      output.appendLine(`\nReformatting ${touchedFiles.size} touched file(s) with clang-format...`);
      execFileSync(CLANG_FORMAT_BIN, ['-i', ...touchedFiles], { stdio: 'ignore' });
    } catch (err) {
      formatFailed = true;
      output.appendLine(`  [warning] clang-format failed: ${err.message}`);
    }
  }

  const summary = `Renamed ${renameCount} function(s) (${totalApplied} edit(s)) across ${touchedFiles.size} file(s).` +
    (applyErrors > 0 ? ` ${applyErrors} file(s) failed to process.` : '') +
    (formatFailed ? ' clang-format failed — touched files may need manual formatting.' : '');
  output.appendLine(`\n${summary}`);
  vscode.window.showInformationMessage(`Enforce Snake Case Function Names: ${summary} Rebuild the project to verify.`);
};
