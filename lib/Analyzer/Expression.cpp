// lib
#include "Support/Result.h"
#include <Analyzer/Analyzer.h>

// stdlib
#include <string>

using namespace AST::Expression;

namespace Analyzer {

// TODO: Check through all ExpressionUnits and check if the types are correct,
// check if the operation is legal for that type etc.

// Also rewrite Parser/Expression.cpp to not care about what kind of operator it
// is using to connect operands, this is what the analyzer should do.

Error ExpressionAnalyzer::evaluateInteger(IntegerLiteral integer,
                                          Type expectedType) {

  RET_ON_FALSE(expectedType.dataType == INT || expectedType == UINT32 ||
                   expectedType == SINT32,
               "evaluateInteger: expected int uint32 sint32.");

  auto value = integer.value;
  RET_ON_TRUE(value.empty(), "evaluateInteger: integer value is empty.");

  // TODO verify that the integer is within 32 bits and consider signed and
  // unsgined.

  return ERU_SUCCESS;
}

Error ExpressionAnalyzer::evaluateString(StringLiteral string,
                                         Type expectedType) {

  RET_ON_FALSE(expectedType == DataType::CHAR ||
                   expectedType == DataType::STRING,
               "evaluateInteger: expected string or character.");

  // TODO evaluate that this contains legal characters (?)

  return ERU_SUCCESS;
}

Result<Type> ExpressionAnalyzer::getIdentifierType(NamedIdentifier identifier) {

  // Check if identifier is declared in current or any parent scope.
  auto variable =
      analyser.getCurrentScope().getVisibleDeclaredVariable(identifier.value);

  RET_ON_FALSE(variable.has_value(),
               "getIdentifierType: use of undeclared identifier.");
  auto aa = variable.value();
  return aa->type;
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

/// TODO: This works very naively. It will let through more tricky expressions
/// that will end up crashing the binary. Look more closely in to this.
Result<Type> ExpressionAnalyzer::EvaluateIndirectionType(
    const AST::Expression::Operand &operand, const NamedIdentifier identifier,
    const Type &type) {

  // This now sees a trimmed down indirection, with only one direction. It can
  // only be one of the following:
  // - No indirection at all.
  // - A single reference &.
  // - One or several dereferences.

  switch (operand.indirection) {

  case OperandIndirection::NONE:
    return Type(type);

  case OperandIndirection::GET_ADDRESS: {
    // Type should be a pointer with depth one above the current depth.
    // If not, something probably went wrong during parsing.
    RET_ON_NOT_EQUAL(operand.steps, 1,
                     "EvaluateIndirectionType: reference step in operand "
                     "exceeds 1, equal to '" +
                         std::to_string(operand.steps) + "'");

    return Type(DataType::INT, true, type.pointerDepth + 1);
  }

  case OperandIndirection::GET_VALUE: {
    RET_ON_FALSE(type.isPointer, "Cannot dereference '" + identifier.value +
                                     "' of type" + type.toPrintableString() +
                                     ".");

    int stepsToTake = type.pointerDepth - operand.steps;
    RET_ON_TRUE(stepsToTake < 0,
                "EvaluateIndirectionType: dereference steps to take '" +
                    std::to_string(operand.steps) +
                    "' exceeds the total pointer depth of" +
                    type.toPrintableString() + ", which has '" +
                    std::to_string(type.pointerDepth) + "' available.");

    bool newTypeIsPointer = stepsToTake != 0;

    return Type(newTypeIsPointer ? DataType::INT : type.dataType,
                newTypeIsPointer, stepsToTake);
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

    const auto &operand = unit->operand;

    if (std::holds_alternative<IntegerLiteral>(operand.operandKind)) {

      if (first) {
        evaluatedType = DataType::INT;
      }

      const auto &integer = std::get<IntegerLiteral>(operand.operandKind);
      RET_ON_FAILURE(evaluateInteger(integer, evaluatedType),
                     "ActOnExpression: failed to evaluate integer.");

    } else if (std::holds_alternative<StringLiteral>(operand.operandKind)) {

      if (first) {
        evaluatedType = DataType::STRING;
      }

      const auto &string = std::get<StringLiteral>(operand.operandKind);
      RET_ON_FAILURE(evaluateString(string, evaluatedType),
                     "ActOnExpression: failed to evaluate string.");

    } else if (std::holds_alternative<NamedIdentifier>(operand.operandKind)) {

      const auto &identifier = std::get<NamedIdentifier>(operand.operandKind);
      auto maybeType = getIdentifierType(identifier);

      RET_ON_FAILURE(maybeType,
                     "ActOnExpression: failed to get identifier type");

      auto type = *maybeType;

      // Get the actual type of the identifer to use in next iteration.
      if (first) {

        auto typeOrError = EvaluateIndirectionType(operand, identifier, type);
        RET_ON_FAILURE(typeOrError,
                       "ActOnExpression: failed to evaluate indirection type.");

        evaluatedType = *typeOrError;

      } else {

        // Type check must consider indirection steps.
        if (type.isPointer && operand.steps != 0) {
          auto stepsLeft = type.pointerDepth - operand.steps;
          if (stepsLeft != 0) {
            RET_ON_NOT_EQUAL(
                type.pointerDepth - stepsLeft, evaluatedType.pointerDepth,
                "ActOnExpression: indirection level do not match. Have '" +
                    identifier.value + "' with '" +
                    std::to_string(type.pointerDepth - stepsLeft) +
                    "'. Expected '" +
                    std::to_string(evaluatedType.pointerDepth) + "'");
          }
          RET_ON_NOT_EQUAL(type.dataType, evaluatedType.dataType,
                           "ActOnExpression: types do not match. Have" +
                               type.toPrintableString(false) +
                               "which does not match with expected" +
                               evaluatedType.toPrintableString(false));
        } else {
          RET_ON_NOT_EQUAL(type, evaluatedType,
                           "ActOnExpression: types do not match. Have" +
                               type.toPrintableString() +
                               "which does not match with expected" +
                               evaluatedType.toPrintableString());
        }
      }
    } else if (std::holds_alternative<AST::Function::FunctionCall *>(
                   operand.operandKind)) {

      auto call = std::get<AST::Function::FunctionCall *>(operand.operandKind);

      // Get the actual type of the function to be called to use in the next
      // iteration.
      if (first) {
        const auto &function =
            analyser.getGlobalScope().getFunctionDeclaration(call->name);

        RET_ON_FALSE(function.has_value(),
                     "ActOnExpression: failed to act on call.");
        evaluatedType = (*function)->type;
      } else {
        RET_ON_FAILURE(analyser.function().ActOnCall(call, evaluatedType),
                       "ActOnExpression: failed to evaluate function call.");
      }
    } else {
      return ERU_FAILURE("ActOnExpression: unexpected operand type");
    }
    first = false;
  }

  expression->evaluatedType = evaluatedType;
  return ERU_SUCCESS;
}

} // namespace Analyzer