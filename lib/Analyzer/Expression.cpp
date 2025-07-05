#include <Analyzer/Analyzer.h>

namespace Analyzer {

// TODO: Check through all ExpressionUnits and check if the types are correct,
// check if the operation is legal for that type etc.

// Also rewrite Parser/Expression.cpp to not care about what kind of operator it
// is using to connect operands, this is what the analyzer should do.

Result<bool> ActOnExpression(AST::Expression::Expression *expression,
                             const AST::Types::Types &type) {}

} // namespace Analyzer