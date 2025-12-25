#include <AST/Expression.h>
#include <Support/Result.h>

namespace Analyzer {

class PrivateAnalyzer;

using namespace AST::Types;

class ExpressionAnalyzer {

  PrivateAnalyzer &analyser;

  Error evaluateInteger(IntegerLiteral integer, Type expectedType);

  Error evaluateString(StringLiteral string, Type expectedType);

  /// Use when we know the type from before.
  Error evaluateIdentifier(NamedIdentifier identifier, Type expectedType);

  /// Use when we need to figure out the type.
  Result<Type> getIdentifierType(NamedIdentifier identifier);

  bool isOperatorAllowed(Lexing::Operator operatorToCheck, Type type);

public:
  ExpressionAnalyzer(PrivateAnalyzer &analyser) : analyser(analyser) {}

  Error ActOn(AST::Expression::Expression *expression);
};

} // namespace Analyzer