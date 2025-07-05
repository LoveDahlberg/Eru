#include "AST/Types.h"
#include "Support/Result.h"
#include <Analyzer/Analyzer.h>

namespace Analyzer {

// TODO: Check through all ExpressionUnits and check if the types are correct,
// check if the operation is legal for that type etc.

// Also rewrite Parser/Expression.cpp to not care about what kind of operator it
// is using to connect operands, this is what the analyzer should do.

Result<bool>
ExpressionAnalyzer::evaluateInteger(AST::Types::IntegerLiteral integer,
                                    AST::Types::Types expectedType) {

  RET_ON_FALSE(expectedType == AST::Types::Types::INT ||
                   expectedType == AST::Types::Types::UINT32 ||
                   expectedType == AST::Types::Types::SINT32,
               "evaluateInteger: expected int uint32 sint32.");

  auto value = integer.value;
  RET_ON_TRUE(value.empty(), "evaluateInteger: integer value is empty.");

  // TODO verify that the integer is within 32 bits and consider signed and
  // unsgined.

  return true;
}

Result<bool>
ExpressionAnalyzer::evaluateString(AST::Types::StringLiteral string,
                                   AST::Types::Types expectedType) {

  RET_ON_FALSE(expectedType == AST::Types::Types::CHAR ||
                   expectedType == AST::Types::Types::STRING,
               "evaluateInteger: expected string or character.");

  // TODO evaluate that this contains legal characters (?)

  return true;
}

Result<bool>
ExpressionAnalyzer::evaluateIdentifier(AST::Types::NamedIdentifier identifier,
                                       AST::Types::Types expectedType) {

  // Check if identifier is declared in current or any parent scope.
  auto *variable = analyser.variable().getDeclaredVariable(identifier);

  RET_ON_EQUAL(variable, nullptr,
               "evaluateIdentifier: use of undeclared variable.");

  // Check that declared variable has the correct type
  RET_ON_NOT_EQUAL(variable->type, expectedType,
                   "evaluateIdentifier: types do not match.");

  return true;
}

// TODO expand this if we need to restrict usages of certain operators and
// types. Like string OR string.
bool ExpressionAnalyzer::isOperatorAllowed(Lexing::Operator operatorToCheck,
                       AST::Types::Types type) {
  switch (operatorToCheck) {
  case Lexing::Operator::MINUS:
  case Lexing::Operator::OR:
  case Lexing::Operator::AND:
  case Lexing::Operator::PLUS: {
    return true;
  }
  }
}

// TODO: Expand this to have automatic promption (like in C, if int is used in
// place of long convert it etc) and type convertions (zero or nullable values
// becomes boolean false and everything else becomes boolean true etc) as
// needed.
Result<bool> ExpressionAnalyzer::ActOn(AST::Expression::Expression *expression,
                                       AST::Types::Types expectedType) {

  bool first = true;

  for (auto unit : expression->ExpressionUnits) {

    if (!first) {
      RET_ON_FALSE(unit->operation.has_value(),
                   "ActOnExpression: expected operator.");

      // TODO add verificaiton of operator between types.
      isOperatorAllowed(*unit->operation, expectedType);
    }

    auto operand = unit->operand;

    if (std::holds_alternative<AST::Types::IntegerLiteral>(operand)) {

      auto integer = std::get<AST::Types::IntegerLiteral>(operand);
      RET_ON_FAILURE(evaluateInteger(integer, expectedType),
                     "ActOnExpression: failed to evaluate integer.");

    } else if (std::holds_alternative<AST::Types::StringLiteral>(operand)) {

      auto string = std::get<AST::Types::StringLiteral>(operand);
      RET_ON_FAILURE(evaluateString(string, expectedType),
                     "ActOnExpression: failed to evaluate string.");

    } else if (std::holds_alternative<AST::Types::NamedIdentifier>(operand)) {

      auto identifier = std::get<AST::Types::NamedIdentifier>(operand);
      RET_ON_FAILURE(evaluateIdentifier(identifier, expectedType),
                     "ActOnExpression: failed to evaluate identifer.");

    } else if (std::holds_alternative<AST::Function::FunctionCall *>(operand)) {

      auto call = std::get<AST::Function::FunctionCall *>(operand);
      RET_ON_FAILURE(analyser.function().ActOnCall(call, expectedType),
                     "ActOnExpression: failed to evaluate function call.");

    } else {
      return {"ActOnExpression: unexpected operand type"};
    }
    first = false;
  }
  return true;
}

} // namespace Analyzer