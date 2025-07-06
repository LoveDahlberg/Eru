#include <AST/Expression.h>
#include <Support/Result.h>

namespace Analyzer {

class PrivateAnalyzer;

class ExpressionAnalyzer {

  PrivateAnalyzer &analyser;

  Error evaluateInteger(AST::Types::IntegerLiteral integer,
                        AST::Types::Types expectedType);

  Error evaluateString(AST::Types::StringLiteral string,
                       AST::Types::Types expectedType);

  /// Use when we know the type from before.
  Error evaluateIdentifier(AST::Types::NamedIdentifier identifier,
                           AST::Types::Types expectedType);

  /// Use when we need to figure out the type.
  Result<AST::Types::Types> getIdentifierType(AST::Types::NamedIdentifier identifier);

  bool isOperatorAllowed(Lexing::Operator operatorToCheck,
                         AST::Types::Types type);

public:
  ExpressionAnalyzer(PrivateAnalyzer &analyser) : analyser(analyser) {}

  Error ActOn(AST::Expression::Expression *expression);
};

} // namespace Analyzer