function collectWorkspaceDiagnostics(vscode) {
  const byUri = new Map();
  for (const [uri, diagnostics] of vscode.languages.getDiagnostics()) {
    if (diagnostics.length > 0) byUri.set(uri, diagnostics);
  }
  return byUri;
}

async function requestQuickFixes(vscode, uri, range) {
  return vscode.commands.executeCommand(
    'vscode.executeCodeActionProvider',
    uri,
    range,
    vscode.CodeActionKind.QuickFix.value
  );
}

// isPreferred is what VS Code uses to pick the top suggestion in the Quick
// Fix lightbulb; when nothing is marked preferred (or it's already been
// applied and a different problem now sits here), just take the next
// available one so every offered fix eventually gets applied, not only the
// first.
function pickQuickFix(actions) {
  if (!actions || actions.length === 0) return null;
  return actions.find(a => a.isPreferred) ?? actions.find(a => a.edit || a.command) ?? null;
}

// Mirrors VS Code's own "apply this code action" behavior: apply the edit (if
// any), then run the follow-up command (if any) - some providers use one, the
// other, or both.
async function applyQuickFix(vscode, action) {
  let didSomething = false;
  if (action.edit) {
    didSomething = (await vscode.workspace.applyEdit(action.edit)) || didSomething;
  }
  if (action.command) {
    await vscode.commands.executeCommand(action.command.command, ...(action.command.arguments ?? []));
    didSomething = true;
  }
  return didSomething;
}

// A single line can carry more than one diagnostic (e.g. two different
// checks, or the same check on two identifiers), and applying one fix can
// reveal another at the same spot. So per line: apply one fix, then re-query
// - since only text at/after this line was touched - until nothing more is
// offered there. maxIterations is just a runaway guard.
async function fixAtLine(vscode, uri, range, output, maxIterations = 25) {
  let applied = 0;
  let currentRange = range;
  for (let i = 0; i < maxIterations; i++) {
    const loc = `${uri.fsPath}:${currentRange.start.line + 1}`;
    let actions;
    try {
      actions = await requestQuickFixes(vscode, uri, currentRange);
    } catch (err) {
      output.appendLine(`    [error] ${loc}: ${err.message}`);
      break;
    }
    const action = pickQuickFix(actions);
    if (!action) break;

    let ok;
    try {
      ok = await applyQuickFix(vscode, action);
    } catch (err) {
      output.appendLine(`    [error] applying "${action.title}" at ${loc}: ${err.message}`);
      break;
    }
    if (!ok) break;
    applied++;

    const remaining = vscode.languages
      .getDiagnostics(uri)
      .filter(d => d.range.start.line === currentRange.start.line);
    if (remaining.length === 0) break;
    currentRange = remaining[0].range;
  }
  return applied;
}

// Fixes are driven bottom-to-top within each document: an edit can only ever
// shift text at or after its own line, so a line recorded before we started
// never becomes stale for any line still ahead of it in this loop.
async function fixDocument(vscode, uri, diagnostics, output) {
  const lines = [...new Set(diagnostics.map(d => d.range.start.line))].sort((a, b) => b - a);
  const rangeByLine = new Map();
  for (const d of diagnostics) {
    if (!rangeByLine.has(d.range.start.line)) rangeByLine.set(d.range.start.line, d.range);
  }

  let applied = 0;
  let linesWithNoFix = 0;
  for (const line of lines) {
    const count = await fixAtLine(vscode, uri, rangeByLine.get(line), output);
    if (count === 0) linesWithNoFix++;
    applied += count;
  }
  return { applied, linesWithNoFix };
}

export default async function ({ vscode, output }) {
  const scopePick = await vscode.window.showQuickPick(
    [
      { label: 'Active editor', detail: 'Apply the suggested quick fix for every problem already flagged in the current file.' },
      {
        label: 'All files with existing diagnostics',
        detail: 'Every file VS Code currently has problems for. Doesn\'t trigger a new scan - run "C/C++: Run Code Analysis" first for files that haven\'t been analyzed yet.'
      }
    ],
    { placeHolder: 'Auto-apply the suggested quick fix for which problems?' }
  );
  if (!scopePick) return;

  let diagnosticsByUri = new Map();
  if (scopePick.label === 'Active editor') {
    const editor = vscode.window.activeTextEditor;
    if (!editor) {
      vscode.window.showWarningMessage('Clang Tidy Auto Apply: no active editor.');
      return;
    }
    const uri = editor.document.uri;
    const diagnostics = vscode.languages.getDiagnostics(uri);
    if (diagnostics.length > 0) diagnosticsByUri.set(uri, diagnostics);
  } else {
    diagnosticsByUri = collectWorkspaceDiagnostics(vscode);
  }

  const totalDiagnostics = [...diagnosticsByUri.values()].reduce((n, d) => n + d.length, 0);
  if (totalDiagnostics === 0) {
    vscode.window.showInformationMessage(
      'Clang Tidy Auto Apply: no problems found. If you expected clang-tidy diagnostics, run "C/C++: Run Code Analysis" (or open the file) first.'
    );
    return;
  }

  const proceed = await vscode.window.showWarningMessage(
    `This will apply every available quick fix for up to ${totalDiagnostics} problem(s) across ${diagnosticsByUri.size} file(s) ` +
    '(lines with more than one applicable fix get all of them, one at a time). ' +
    'Make sure your work is committed or stashed so you can review the diff. Continue?',
    { modal: true },
    'Apply'
  );
  if (proceed !== 'Apply') return;

  output.show();
  output.appendLine(`Applying suggested quick fixes across ${diagnosticsByUri.size} file(s)...`);

  let totalApplied = 0;
  let totalLinesWithNoFix = 0;
  let filesChanged = 0;

  for (const [uri, diagnostics] of diagnosticsByUri) {
    output.appendLine(`  ${uri.fsPath} (${diagnostics.length} problem(s))`);
    const { applied, linesWithNoFix } = await fixDocument(vscode, uri, diagnostics, output);
    if (applied > 0) {
      const doc = await vscode.workspace.openTextDocument(uri);
      if (doc.isDirty) await doc.save();
      filesChanged++;
    }
    totalApplied += applied;
    totalLinesWithNoFix += linesWithNoFix;
    output.appendLine(`    applied ${applied} fix(es); ${linesWithNoFix} line(s) had no quick fix available`);
  }

  const summary = `Applied ${totalApplied} fix(es) across ${filesChanged} file(s); ${totalLinesWithNoFix} line(s) had no quick fix available.`;
  output.appendLine(`\n${summary}`);
  vscode.window.showInformationMessage(`Clang Tidy Auto Apply: ${summary}`);
};
