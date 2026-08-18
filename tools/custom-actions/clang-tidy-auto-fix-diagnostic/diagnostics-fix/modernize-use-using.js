// typedef -> using is a straight text substitution clang-tidy has already
// worked out from the AST.
module.exports = function computeEdits(diagnostic) {
  return diagnostic.replacements;
};
