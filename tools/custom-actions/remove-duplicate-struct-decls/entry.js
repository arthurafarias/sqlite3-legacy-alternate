const { execFileSync } = require('child_process');
const fs = require('fs');
const path = require('path');

const CLANG_BIN = 'clang';
const CXX_STD = '-std=c++20';
const HEADER_EXTENSIONS = new Set(['.h', '.hpp', '.hh']);
const SKIP_DIR_NAMES = new Set(['.git', 'node_modules', 'build', 'out', '.vscode-test', 'CMakeFiles']);
const DUPLICATE_LINE_RE = /^(struct|class|union|enum)\s+[A-Za-z_]\w*\s*;$/;

// readability-redundant-declaration (clang-tidy) only matches VarDecl/FunctionDecl,
// so struct/class/union/enum forward declarations are invisible to stock clang-tidy
// checks. Instead we ask clang itself for the AST (which already resolves the
// redeclaration chain via `previousDecl`) and do the redundancy check ourselves.
const TAG_KINDS = new Set(['RecordDecl', 'CXXRecordDecl', 'EnumDecl']);
const TRANSPARENT_KINDS = new Set(['TranslationUnitDecl', 'NamespaceDecl', 'LinkageSpecDecl']);

function findSrcRoot(filePath) {
  let dir = path.dirname(filePath);
  while (true) {
    if (path.basename(dir) === 'src') return dir;
    const parent = path.dirname(dir);
    if (parent === dir) return path.dirname(filePath);
    dir = parent;
  }
}

function listHeaders(root) {
  const results = [];
  const stack = [root];
  while (stack.length) {
    const dir = stack.pop();
    let entries;
    try {
      entries = fs.readdirSync(dir, { withFileTypes: true });
    } catch {
      continue;
    }
    for (const entry of entries) {
      if (entry.isDirectory()) {
        if (!SKIP_DIR_NAMES.has(entry.name)) stack.push(path.join(dir, entry.name));
      } else if (HEADER_EXTENSIONS.has(path.extname(entry.name))) {
        results.push(path.join(dir, entry.name));
      }
    }
  }
  return results;
}

function dumpAst(filePath) {
  const args = [
    '-Xclang', '-ast-dump=json', '-fsyntax-only',
    '-x', 'c++', CXX_STD,
    `-I${findSrcRoot(filePath)}`,
    filePath
  ];
  try {
    const stdout = execFileSync(CLANG_BIN, args, {
      encoding: 'utf8',
      maxBuffer: 64 * 1024 * 1024,
      stdio: ['ignore', 'pipe', 'pipe']
    });
    return JSON.parse(stdout);
  } catch (err) {
    if (err.stdout) {
      try {
        return JSON.parse(err.stdout);
      } catch {
        // fall through to the error below
      }
    }
    throw new Error(err.stderr || err.message);
  }
}

// clang's JSON AST dump omits `file`/`line` on a location object whenever they
// are unchanged from the last location it printed (in traversal order: a node's
// own `loc`, then `range.begin`, then `range.end`, then its children). We have
// to replay that same running state to know which file/line an omitted entry
// actually refers to.
function findRedundantTagLines(ast, targetFile) {
  const candidates = [];
  let currentFile = null;
  let currentLine = null;

  function resolveLoc(locObj) {
    if (!locObj) return null;
    if (locObj.file !== undefined) currentFile = locObj.file;
    if (locObj.line !== undefined) currentLine = locObj.line;
    return { file: currentFile, line: currentLine };
  }

  function visit(node, opaque) {
    if (!node || typeof node !== 'object') return;

    resolveLoc(node.loc);
    const rangeBegin = resolveLoc(node.range && node.range.begin);
    const rangeEnd = resolveLoc(node.range && node.range.end);

    if (TAG_KINDS.has(node.kind) && !opaque && node.previousDecl && node.completeDefinition !== true) {
      if (rangeBegin && rangeEnd && rangeBegin.file === targetFile && rangeBegin.line === rangeEnd.line) {
        candidates.push(rangeBegin.line);
      }
    }

    // Anything nested inside a non-transparent decl (a class body, a function
    // body, ...) is not a top-level redeclaration - e.g. skip the injected
    // class-name that clang inserts as a "child" of every class definition.
    const childOpaque = TRANSPARENT_KINDS.has(node.kind) ? opaque : true;
    if (Array.isArray(node.inner)) {
      for (const child of node.inner) visit(child, childOpaque);
    }
  }

  visit(ast, false);
  return candidates;
}

function analyzeFile(filePath) {
  const ast = dumpAst(filePath);
  const lineNumbers = [...new Set(findRedundantTagLines(ast, filePath))].sort((a, b) => a - b);

  const sourceLines = fs.readFileSync(filePath, 'utf8').split(/\r?\n/);
  const toDelete = [];
  const skipped = [];
  for (const lineNo of lineNumbers) {
    const text = sourceLines[lineNo - 1];
    if (text !== undefined && DUPLICATE_LINE_RE.test(text.trim())) {
      toDelete.push(lineNo);
    } else {
      skipped.push(lineNo);
    }
  }
  return { toDelete, skipped };
}

module.exports = async function ({ vscode, workspaceRoot, output }) {
  try {
    execFileSync(CLANG_BIN, ['--version'], { stdio: 'ignore' });
  } catch {
    vscode.window.showErrorMessage(`Remove Duplicate Struct Declarations: "${CLANG_BIN}" was not found on PATH.`);
    return;
  }

  const scopePick = await vscode.window.showQuickPick(
    [
      { label: 'Current file', detail: 'Only the active editor' },
      { label: 'Entire workspace', detail: `Every .h/.hpp/.hh file under ${workspaceRoot}` }
    ],
    { placeHolder: 'Scan which files for duplicate struct/class/union/enum declarations?' }
  );
  if (!scopePick) return;

  let targetFiles;
  if (scopePick.label === 'Current file') {
    const editor = vscode.window.activeTextEditor;
    const filePath = editor?.document?.uri?.fsPath;
    if (!filePath || !HEADER_EXTENSIONS.has(path.extname(filePath))) {
      vscode.window.showWarningMessage('Remove Duplicate Struct Declarations: no header file is active.');
      return;
    }
    targetFiles = [filePath];
  } else {
    targetFiles = listHeaders(workspaceRoot);
  }

  output.appendLine(`Scanning ${targetFiles.length} file(s) for duplicate struct/class/union/enum declarations...`);

  const edit = new vscode.WorkspaceEdit();
  let filesModified = 0;
  let linesRemoved = 0;
  let filesSkippedOnError = 0;
  let linesNeedingReview = 0;

  for (const filePath of targetFiles) {
    let result;
    try {
      result = analyzeFile(filePath);
    } catch (err) {
      filesSkippedOnError++;
      output.appendLine(`  [error] ${filePath}: ${err.message}`);
      continue;
    }

    if (result.skipped.length > 0) {
      linesNeedingReview += result.skipped.length;
      output.appendLine(`  [review] ${filePath}: line(s) ${result.skipped.join(', ')} look redundant but don't match the plain "kind Name;" shape - left untouched.`);
    }

    if (result.toDelete.length === 0) continue;

    const uri = vscode.Uri.file(filePath);
    const doc = await vscode.workspace.openTextDocument(uri);
    for (const lineNo of result.toDelete) {
      edit.delete(uri, doc.lineAt(lineNo - 1).rangeIncludingLineBreak);
    }
    filesModified++;
    linesRemoved += result.toDelete.length;
    output.appendLine(`  [fix] ${filePath}: removing line(s) ${result.toDelete.join(', ')}`);
  }

  if (linesRemoved > 0) {
    await vscode.workspace.applyEdit(edit);
  }

  const summary = `Removed ${linesRemoved} duplicate declaration(s) across ${filesModified} file(s).` +
    (linesNeedingReview > 0 ? ` ${linesNeedingReview} line(s) need manual review.` : '') +
    (filesSkippedOnError > 0 ? ` ${filesSkippedOnError} file(s) failed to parse.` : '');
  output.appendLine(summary);
  output.show();
  vscode.window.showInformationMessage(`Remove Duplicate Struct Declarations: ${summary}`);
};
