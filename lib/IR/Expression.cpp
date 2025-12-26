#include "Support/Result.h"
#include <IR/IRGenerator.h>

// llvm
#include <llvm-20/llvm/Support/Error.h>
#include <llvm/IR/Constants.h>

namespace IR {

Result<llvm::Value *>
IRGenerator::getOperand(Expression::ExpressionUnit *expressionUnit) {
  if (std::holds_alternative<Types::IntegerLiteral>(expressionUnit->operand.operandKind)) {
    auto integerLiteral =
        std::get<Types::IntegerLiteral>(expressionUnit->operand.operandKind);

    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(module.getContext()),
                                  stoi(integerLiteral.value));
  }
  if (std::holds_alternative<Function::FunctionCall *>(
          expressionUnit->operand.operandKind)) {
    auto call = std::get<Function::FunctionCall *>(expressionUnit->operand.operandKind);

    return handle(*call);
  }
  if (std::holds_alternative<Types::NamedIdentifier>(expressionUnit->operand.operandKind)) {
    auto identifier = std::get<Types::NamedIdentifier>(expressionUnit->operand.operandKind);

    // Make sure the variable value being pointed to is used here, as the
    // expression is done on the value itself.
    // TODO: If pointer arithmetic is added, extend this logic to handle it.
    auto result =
        scopeHandler.getCurrent().getVisibleDeclaredVariable(identifier.value);

    return result.has_value() ? result->getValue(builder) : nullptr;
  }
  if (std::holds_alternative<Types::StringLiteral>(expressionUnit->operand.operandKind)) {
    llvm::report_fatal_error(
        "IRExpression: getOperand: strings are not implemented yet.");
  }
  llvm::report_fatal_error(
      "IRExpression: getOperand: encountered unimplemented type");
}

// TODO: Only handle integer subtraction and addition for now, add more
// operations after semantic parsing is implemented.
Result<llvm::Value *> IRGenerator::handle(Expression::Expression &AST) {
  RET_ON_TRUE(builder == nullptr, "IRGenerator: Expression: builder is null");

  llvm::Value *totalValue = nullptr;
  bool firstExpressionUnit = true;
  for (auto expressionUnit : AST.ExpressionUnits) {

    // Check operand being held.
    auto maybeOperand = getOperand(expressionUnit);
    RET_ON_FAILURE(maybeOperand, "IRGenerator: Expression: get operand failed");

    auto operand = *maybeOperand;

    if (firstExpressionUnit) {
      firstExpressionUnit = false;
      totalValue = operand;
      continue;
    }

    switch (*expressionUnit->operation) {
    case Lexing::Operator::PLUS: {
      totalValue = builder->CreateAdd(totalValue, operand);
      break;
    }

    case Lexing::Operator::MINUS: {
      totalValue = builder->CreateSub(totalValue, operand);
      break;
    }
    case Lexing::Operator::OR:
    case Lexing::Operator::AND:
      break;
    }
  }

  return totalValue;
}

} // namespace IR