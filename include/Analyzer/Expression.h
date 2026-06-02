#include <AST/Expression.h>
#include <Support/Result.h>

namespace Analyzer {

class PrivateAnalyzer;

using namespace AST::Types;

class ExpressionAnalyzer {

  PrivateAnalyzer &analyser;

  Error evaluateInteger(IntegerLiteral integer, Type expectedType);

  Error evaluateString(StringLiteral string, Type expectedType);

  /// Use when we need to figure out the type.
  Result<Type> getIdentifierType(NamedIdentifier identifier);

  bool isOperatorAllowed(Lexing::Operator operatorToCheck, Type type);

  Result<Type> EvaluateIndirectionType(const AST::Expression::Operand &operand,
                            const NamedIdentifier identifier, const Type &type);

public:
  ExpressionAnalyzer(PrivateAnalyzer &analyser) : analyser(analyser) {}

  Error ActOnExpression(AST::Expression::Expression *expression);
};

} // namespace Analyzer