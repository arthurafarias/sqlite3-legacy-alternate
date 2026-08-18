// Pure deletion of a redundant "(void)" parameter list - always safe when
// clang-tidy reports it.
module.exports = function computeEdits(diagnostic) {
  return diagnostic.replacements;
};
