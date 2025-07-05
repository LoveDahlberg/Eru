

#include "AST/Function.h"
#include "AST/Types.h"
#include "Lexer/Tokens.h"
#include <AST/Expression.h>
#include <Support/Result.h>

namespace Analyzer {

class PrivateAnalyzer;

class ExpressionAnalyzer {

  PrivateAnalyzer &analyser;

  Result<bool> evaluateInteger(AST::Types::IntegerLiteral integer,
                               AST::Types::Types expectedType);

  Result<bool> evaluateString(AST::Types::StringLiteral string,
                              AST::Types::Types expectedType);

  Result<bool> evaluateIdentifier(AST::Types::NamedIdentifier identifier,
                                  AST::Types::Types expectedType);

  bool isOperatorAllowed(Lexing::Operator operatorToCheck,
                         AST::Types::Types type);

public:
  ExpressionAnalyzer(PrivateAnalyzer &analyser) : analyser(analyser) {}

  Result<bool> ActOn(AST::Expression::Expression *expression,
                     AST::Types::Types expectedType);
};

} // namespace Analyzer