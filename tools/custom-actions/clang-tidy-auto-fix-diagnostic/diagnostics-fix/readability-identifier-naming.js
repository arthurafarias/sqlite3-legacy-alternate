// Renames a single identifier occurrence. Scanning every translation unit in
// the compile database means the same header-level declaration is reported
// once per TU that includes it - entry.js dedupes the resulting identical
// Replacements before building the WorkspaceEdit, so it's safe to hand back
// every occurrence here unfiltered.
module.exports = function computeEdits(diagnostic) {
  return diagnostic.replacements;
};
