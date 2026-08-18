// Converting to a trailing return type is a two-part edit: insert "auto " at
// the front and append "-> OldType" after the parameter list. clang-tidy
// already pairs those two Replacements for the same diagnostic, so keep them
// together as a unit rather than trying to re-derive one from the other.
module.exports = function computeEdits(diagnostic) {
  return diagnostic.replacements;
};
