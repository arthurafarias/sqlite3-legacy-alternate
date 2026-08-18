import { execFileSync } from 'child_process';
import { readdirSync, realpathSync, readFileSync, existsSync, rmSync } from 'fs';
import { tmpdir } from 'os';
import { join } from 'path';
import { load } from 'js-yaml';

const CLANG_TIDY_BIN = 'clang-tidy';
const CLANG_FORMAT_BIN = 'clang-format';
const FIXES_DIR = join(__dirname, 'diagnostics-fix');

// clang-tidy's --export-fixes output is real YAML (LLVM's yaml::Output), so
// parse it as such rather than pattern-matching the field layout by hand.
function parseExportedFixes(yamlText) {
  const doc = load(yamlText);
  const diagnostics = doc?.Diagnostics ?? [];
  return diagnostics.map(d => ({
    name: d.DiagnosticName,
    replacements: (d.DiagnosticMessage?.Replacements ?? []).map(r => ({
      filePath: r.FilePath,
      offset: r.Offset,
      length: r.Length,
      replacementText: r.ReplacementText ?? ''
    }))
  }));
}

function loadFixModules() {
  const modules = new Map();
  for (const entry of readdirSync(FIXES_DIR, { withFileTypes: true })) {
    if (!entry.isFile() || !entry.name.endsWith('.js')) continue;
    modules.set(entry.name.slice(0, -3), require(join(FIXES_DIR, entry.name)));
  }
  return modules;
}

function realPath(p) {
  try {
    return realpathSync(p);
  } catch {
    return p;
  }
}

function loadCompileDbFiles(compileDbPath) {
  const entries = JSON.parse(readFileSync(compileDbPath, 'utf8'));
  const seen = new Set();
  const files = [];
  for (const entry of entries) {
    if (!entry.file) continue;
    const rp = realPath(entry.file);
    if (seen.has(rp) || !existsSync(rp)) continue;
    seen.add(rp);
    files.push(rp);
  }
  return files.sort();
}

// No --config override here: the whole point of this action is to defer to
// whatever .clang-tidy the workspace already has (CheckOptions like naming
// case are picked up by clang-tidy's own upward directory search), only
// narrowing --checks to the ones the user picked.
function runExportFixes(filePath, compileDbDir, checksArg, exportPath) {
  const args = [
    `-p=${compileDbDir}`,
    `--checks=-*,${checksArg}`,
    `--export-fixes=${exportPath}`,
    filePath
  ];
  try {
    execFileSync(CLANG_TIDY_BIN, args, {
      encoding: 'utf8',
      maxBuffer: 64 * 1024 * 1024,
      stdio: ['ignore', 'pipe', 'pipe']
    });
  } catch (err) {
    // clang-tidy can exit non-zero on diagnostics/compile issues even though
    // --export-fixes was still written; only treat it as fatal if nothing came out.
    if (!existsSync(exportPath)) throw new Error(err.stderr || err.message);
  }
}

export default async function ({ vscode, workspaceRoot, output }) {
  try {
    execFileSync(CLANG_TIDY_BIN, ['--version'], { stdio: 'ignore' });
  } catch {
    vscode.window.showErrorMessage(`Clang Tidy Auto Fix Diagnostics: "${CLANG_TIDY_BIN}" was not found on PATH.`);
    return;
  }

  const fixModules = loadFixModules();
  const checkNames = [...fixModules.keys()].sort();
  if (checkNames.length === 0) {
    vscode.window.showErrorMessage('Clang Tidy Auto Fix Diagnostics: no fix implementations found under diagnostics-fix/.');
    return;
  }

  const compileDbDir = join(workspaceRoot, 'build');
  const compileDbPath = join(compileDbDir, 'compile_commands.json');
  if (!existsSync(compileDbPath)) {
    vscode.window.showErrorMessage(
      `Clang Tidy Auto Fix Diagnostics: no compile_commands.json found at ${compileDbPath}. ` +
      'Configure the CMake build with CMAKE_EXPORT_COMPILE_COMMANDS=ON first.'
    );
    return;
  }

  const checkPicks = await vscode.window.showQuickPick(
    checkNames.map(name => ({ label: name, picked: true })),
    { canPickMany: true, placeHolder: 'Which clang-tidy checks should be auto-fixed?' }
  );
  if (!checkPicks || checkPicks.length === 0) return;
  const selectedChecks = checkPicks.map(p => p.label);

  const scopePick = await vscode.window.showQuickPick(
    [
      { label: 'Entire workspace', detail: 'Every translation unit in the compile database. Recommended, since fixes are deduped across files.' },
      { label: 'Current file only', detail: 'Only the active editor (must be a .cpp translation unit already in the compile database).' }
    ],
    { placeHolder: 'Scan which files for the selected diagnostics?' }
  );
  if (!scopePick) return;

  let targetFiles;
  if (scopePick.label === 'Current file only') {
    const filePath = vscode.window.activeTextEditor?.document?.uri?.fsPath;
    if (!filePath) {
      vscode.window.showWarningMessage('Clang Tidy Auto Fix Diagnostics: no active editor.');
      return;
    }
    const rp = realPath(filePath);
    if (!loadCompileDbFiles(compileDbPath).includes(rp)) {
      vscode.window.showWarningMessage('Clang Tidy Auto Fix Diagnostics: the active file is not a translation unit in the compile database.');
      return;
    }
    targetFiles = [rp];
  } else {
    targetFiles = loadCompileDbFiles(compileDbPath);
  }

  const checksArg = selectedChecks.join(',');
  const exportPath = join(tmpdir(), `clang-tidy-auto-fix-diagnostic-${process.pid}.yaml`);

  output.show();
  output.appendLine(`Scanning ${targetFiles.length} file(s) for: ${selectedChecks.join(', ')}`);

  // filePath -> accepted [{start, end, text}], used to dedupe/detect conflicts
  // before ever touching a WorkspaceEdit (which rejects overlapping ranges outright).
  const acceptedRanges = new Map();
  // filePath -> [{offset, length, replacementText}]
  const editsByFile = new Map();
  const countsByCheck = new Map();
  let duplicateCount = 0;
  let conflictCount = 0;
  let scanErrors = 0;

  function tryAcceptEdit(filePath, offset, length, replacementText) {
    const end = offset + length;
    let ranges = acceptedRanges.get(filePath);
    if (!ranges) {
      ranges = [];
      acceptedRanges.set(filePath, ranges);
    }
    for (const r of ranges) {
      if (offset < r.end && end > r.start) {
        if (r.start === offset && r.end === end && r.text === replacementText) {
          duplicateCount++;
        } else {
          conflictCount++;
          output.appendLine(`  [conflict] ${filePath}: edit at ${offset}-${end} overlaps one already accepted at ${r.start}-${r.end} - skipped.`);
        }
        return;
      }
    }
    ranges.push({ start: offset, end, text: replacementText });
    let edits = editsByFile.get(filePath);
    if (!edits) {
      edits = [];
      editsByFile.set(filePath, edits);
    }
    edits.push({ offset, length, replacementText });
  }

  for (const [i, filePath] of targetFiles.entries()) {
    output.appendLine(`  [${i + 1}/${targetFiles.length}] scanning ${filePath}`);
    rmSync(exportPath, { force: true });
    try {
      runExportFixes(filePath, compileDbDir, checksArg, exportPath);
    } catch (err) {
      scanErrors++;
      output.appendLine(`    [error] ${err.message}`);
      continue;
    }
    if (!existsSync(exportPath)) continue;

    let diagnostics;
    try {
      diagnostics = parseExportedFixes(readFileSync(exportPath, 'utf8'));
    } catch (err) {
      scanErrors++;
      output.appendLine(`    [error] failed to parse export-fixes output: ${err.message}`);
      continue;
    }

    for (const diagnostic of diagnostics) {
      const computeEdits = fixModules.get(diagnostic.name);
      if (!computeEdits) continue; // shouldn't happen: --checks was restricted to selectedChecks
      const replacements = computeEdits(diagnostic) || [];
      if (replacements.length > 0) {
        countsByCheck.set(diagnostic.name, (countsByCheck.get(diagnostic.name) || 0) + 1);
      }
      for (const r of replacements) {
        tryAcceptEdit(realPath(r.filePath), r.offset, r.length, r.replacementText);
      }
    }
  }
  rmSync(exportPath, { force: true });

  const totalEdits = [...editsByFile.values()].reduce((n, edits) => n + edits.length, 0);
  if (totalEdits === 0) {
    const suffix = scanErrors > 0 ? ` (${scanErrors} file(s) failed to scan — see output)` : '';
    vscode.window.showInformationMessage(`Clang Tidy Auto Fix Diagnostics: no auto-fixable occurrences found${suffix}.`);
    return;
  }

  output.appendLine(`\nFound ${totalEdits} fix(es) across ${editsByFile.size} file(s):`);
  for (const [name, count] of [...countsByCheck.entries()].sort()) {
    output.appendLine(`  ${name}: ${count}`);
  }
  if (duplicateCount > 0) output.appendLine(`${duplicateCount} duplicate edit(s) seen from multiple translation units were merged.`);
  if (conflictCount > 0) output.appendLine(`${conflictCount} conflicting edit(s) were skipped — see [conflict] lines above.`);
  if (scanErrors > 0) output.appendLine(`${scanErrors} file(s) failed to scan — see [error] lines above.`);

  const proceed = await vscode.window.showWarningMessage(
    `This will apply ${totalEdits} clang-tidy fix(es) across ${editsByFile.size} file(s) for: ${selectedChecks.join(', ')}. ` +
    'Make sure your work is committed or stashed so you can review the diff. Continue?',
    { modal: true },
    'Apply'
  );
  if (proceed !== 'Apply') return;

  output.appendLine('\nApplying fixes...');
  const edit = new vscode.WorkspaceEdit();
  const touchedFiles = new Set();

  for (const [filePath, edits] of editsByFile) {
    const uri = vscode.Uri.file(filePath);
    let doc;
    try {
      doc = await vscode.workspace.openTextDocument(uri);
    } catch (err) {
      output.appendLine(`  [error] could not open ${filePath}: ${err.message}`);
      continue;
    }
    for (const e of edits) {
      const range = new vscode.Range(doc.positionAt(e.offset), doc.positionAt(e.offset + e.length));
      edit.replace(uri, range, e.replacementText);
    }
    touchedFiles.add(filePath);
  }
  await vscode.workspace.applyEdit(edit);

  let formatFailed = false;
  if (touchedFiles.size > 0) {
    try {
      execFileSync(CLANG_FORMAT_BIN, ['--version'], { stdio: 'ignore' });
      output.appendLine(`Reformatting ${touchedFiles.size} touched file(s) with clang-format...`);
      execFileSync(CLANG_FORMAT_BIN, ['-i', ...touchedFiles], { stdio: 'ignore' });
    } catch (err) {
      formatFailed = true;
      output.appendLine(`  [warning] clang-format failed: ${err.message}`);
    }
  }

  const summary = `Applied ${totalEdits} fix(es) across ${touchedFiles.size} file(s).` +
    (formatFailed ? ' clang-format failed — touched files may need manual formatting.' : '');
  output.appendLine(`\n${summary}`);
  vscode.window.showInformationMessage(`Clang Tidy Auto Fix Diagnostics: ${summary} Rebuild the project to verify.`);
};
