#include <AST/Expression.h>

// llvm
#include <llvm/IR/Constants.h>
#include <optional>
#include <variant>

namespace AST::Expression {

llvm::Value *getOperand(codeGenItems &items, ExpressionUnit *expressionUnit) {
  if (std::holds_alternative<Types::IntegerLiteral>(expressionUnit->operand)) {
    auto integerLiteral =
        std::get<Types::IntegerLiteral>(expressionUnit->operand);

    return llvm::ConstantInt::get(
        llvm::Type::getInt32Ty(items.module.getContext()),
        stoi(integerLiteral.value));
  }
  // TODO Add the rest
  return nullptr;
}

// TODO: Only handle integer subtraction and addition for now, add more
// operations after semantic parsing is implemented.
llvm::Value *Expression::codegen(codeGenItems &items) {
  if (items.builder == nullptr) {
    return nullptr;
  }

  llvm::Value *totalValue = nullptr;
  bool firstExpressionUnit = true;
  for (auto expressionUnit : ExpressionUnits) {

    // Check operand being held.
    llvm::Value *operand = getOperand(items, expressionUnit);

    if (firstExpressionUnit) {
      firstExpressionUnit = false;
      totalValue = operand;
      continue;
    }
    if (std::holds_alternative<ArithmeticOperator>(
            *expressionUnit->operation)) {
      auto arithmeticOperator =
          std::get<ArithmeticOperator>(*expressionUnit->operation);

      switch (arithmeticOperator) {
      case ArithmeticOperator::PLUS: {
        totalValue = items.builder->CreateAdd(totalValue, operand);
        break;
      }

      case ArithmeticOperator::MINUS: {
        totalValue = items.builder->CreateSub(totalValue, operand);
        break;
      }
      }
    }
  }

  return totalValue;
}

} // namespace AST::Expression