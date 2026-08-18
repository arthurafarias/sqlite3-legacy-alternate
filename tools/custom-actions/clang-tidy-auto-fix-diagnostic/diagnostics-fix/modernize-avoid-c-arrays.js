// clang-tidy only attaches Replacements to this diagnostic when it found an
// unambiguous std::array/std::vector rewrite - multi-dimensional and
// function-parameter C arrays get flagged without a fix-it, so an empty
// Replacements list here just means "nothing safe to apply".
module.exports = function computeEdits(diagnostic) {
  return diagnostic.replacements;
};
