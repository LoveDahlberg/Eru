#include <Analyzer/Analyzer.h>

namespace Analyzer {

// TODO: Check through all ExpressionUnits and check if the types are correct,
// check if the operation is legal for that type etc.

// Also rewrite Parser/Expression.cpp to not care about what kind of operator it
// is using to connect operands, this is what the analyzer should do.

Error ExpressionAnalyzer::evaluateInteger(IntegerLiteral integer,
                                          Type expectedType) {

  RET_ON_FALSE(expectedType.dataType == INT ||
                   expectedType == UINT32 ||
                   expectedType == SINT32,
               "evaluateInteger: expected int uint32 sint32.");

  auto value = integer.value;
  RET_ON_TRUE(value.empty(), "evaluateInteger: integer value is empty.");

  // TODO verify that the integer is within 32 bits and consider signed and
  // unsgined.

  return SUCCESS;
}

Error ExpressionAnalyzer::evaluateString(StringLiteral string,
                                         Type expectedType) {

  RET_ON_FALSE(expectedType == DataType::CHAR ||
                   expectedType == DataType::STRING,
               "evaluateInteger: expected string or character.");

  // TODO evaluate that this contains legal characters (?)

  return SUCCESS;
}

Result<Type>
ExpressionAnalyzer::getIdentifierType(NamedIdentifier identifier) {

  // Check if identifier is declared in current or any parent scope.
  auto variable =
      analyser.getCurrentScope().getVisibleDeclaredVariable(identifier.value);

  RET_ON_FALSE(variable.has_value(),
               "getIdentifierType: use of undeclared identifier.");

  return (*variable)->type;
}

Error ExpressionAnalyzer::evaluateIdentifier(
    NamedIdentifier identifier, Type expectedType) {

  // Get the type of the identifier
  auto type = getIdentifierType(identifier);

  RET_ON_FAILURE(type, "evaluateIdentifier: failed to get identifier type");

  // Check that declared variable has the correct type
  RET_ON_NOT_EQUAL(*type, expectedType,
                   "evaluateIdentifier: types do not match.");

  return SUCCESS;
}

// TODO expand this if we need to restrict usages of certain operators and
// types. Like string OR string.
bool ExpressionAnalyzer::isOperatorAllowed(Lexing::Operator operatorToCheck,
                                           Type type) {
  switch (operatorToCheck) {
  case Lexing::Operator::MINUS:
  case Lexing::Operator::OR:
  case Lexing::Operator::AND:
  case Lexing::Operator::PLUS: {
    return true;
  }
  }
}

// TODO: Expand this to have automatic promoption (like in C, if int is used in
// place of long convert it etc) and type convertions (zero or nullable values
// becomes boolean false and everything else becomes boolean true etc) as
// needed.
Error ExpressionAnalyzer::ActOn(AST::Expression::Expression *expression) {

  // We don't know the type before parsing. For now, all types in the expression
  // much match. Assume that this type is the first seen.
  Type evaluatedType = NONE;

  bool first = true;
  for (auto unit : expression->ExpressionUnits) {

    if (!first) {
      RET_ON_FALSE(unit->operation.has_value(),
                   "ActOnExpression: expected operator.");

      // TODO add verificaiton of operator between types.
      isOperatorAllowed(*unit->operation, evaluatedType);
    }

    auto operand = unit->operand;

    if (std::holds_alternative<IntegerLiteral>(operand)) {

      if (first) {
        evaluatedType = DataType::INT;
      }

      auto integer = std::get<IntegerLiteral>(operand);
      RET_ON_FAILURE(evaluateInteger(integer, evaluatedType),
                     "ActOnExpression: failed to evaluate integer.");

    } else if (std::holds_alternative<StringLiteral>(operand)) {

      if (first) {
        evaluatedType = DataType::STRING;
      }

      auto string = std::get<StringLiteral>(operand);
      RET_ON_FAILURE(evaluateString(string, evaluatedType),
                     "ActOnExpression: failed to evaluate string.");

    } else if (std::holds_alternative<NamedIdentifier>(operand)) {

      auto identifier = std::get<NamedIdentifier>(operand);

      // Get the actual type of the identifer to use in next iteration.
      if (first) {
        auto type = getIdentifierType(identifier);

        RET_ON_FAILURE(type,
                       "evaluateIdentifier: failed to get identifier type");
        evaluatedType = *type;
      } else {
        RET_ON_FAILURE(evaluateIdentifier(identifier, evaluatedType),
                       "ActOnExpression: failed to evaluate identifer.");
      }

    } else if (std::holds_alternative<AST::Function::FunctionCall *>(operand)) {

      auto call = std::get<AST::Function::FunctionCall *>(operand);

      // Get the actual type of the function to be called to use in the next
      // iteration.
      if (first) {
        auto function =
            analyser.getGlobalScope().getFunctionDeclaration(call->name);

        RET_ON_FALSE(function.has_value(),
                     "ActOnExpression: failed to act on call.");
        evaluatedType = (*function)->type;
      } else {
        RET_ON_FAILURE(analyser.function().ActOnCall(call, evaluatedType),
                       "ActOnExpression: failed to evaluate function call.");
      }

    } else {
      return FAILURE("ActOnExpression: unexpected operand type");
    }
    first = false;
  }

  expression->evaluatedType = evaluatedType;
  return SUCCESS;
}

} // namespace Analyzer