// Which cast kind (static_cast/reinterpret_cast/const_cast/...) is correct
// depends on the operand's resolved type, which only clang's Sema can know -
// reuse the Replacements clang-tidy already derived from the AST instead of
// guessing the right cast from source text.
module.exports = function computeEdits(diagnostic) {
  return diagnostic.replacements;
};
