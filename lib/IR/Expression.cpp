#include "Support/Result.h"
#include <IR/IRGenerator.h>

// llvm
#include <llvm-20/llvm/Support/Error.h>
#include <llvm/IR/Constants.h>

namespace IR {

Result<llvm::Value *>
IRGenerator::getOperand(Expression::ExpressionUnit *expressionUnit) {
  const auto &operand = expressionUnit->operand;

  if (std::holds_alternative<Types::IntegerLiteral>(operand.operandKind)) {
    auto integerLiteral = std::get<Types::IntegerLiteral>(operand.operandKind);

    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(module.getContext()),
                                  stoi(integerLiteral.value));
  }
  if (std::holds_alternative<Function::FunctionCall *>(operand.operandKind)) {
    auto call = std::get<Function::FunctionCall *>(operand.operandKind);

    return handle(*call);
  }
  if (std::holds_alternative<Types::NamedIdentifier>(operand.operandKind)) {
    auto identifier = std::get<Types::NamedIdentifier>(operand.operandKind);

    // TODO: If pointer arithmetic is added, extend this logic to handle it.
    auto maybeScopeVariable =
        scopeHandler.getCurrent().getVisibleDeclaredVariable(identifier.value);

    if (!maybeScopeVariable.has_value()) {
      return nullptr;
    }

    auto* scopeVariable = *maybeScopeVariable;
    
    switch (operand.indirection) {
    case AST::Expression::OperandIndirection::NONE:
      return scopeVariable->getValue(builder);
    case AST::Expression::OperandIndirection::GET_ADDRESS: {
      return scopeVariable->getAddress(builder);
    }
    case AST::Expression::OperandIndirection::GET_VALUE:
      return scopeVariable->dereferenceExpression(builder, operand.steps);
    }
  }
  if (std::holds_alternative<Types::StringLiteral>(operand.operandKind)) {
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