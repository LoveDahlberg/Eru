#include <IR/IRGenerator.h>

// llvm
#include <llvm/IR/Constants.h>

namespace IR {

llvm::Value *
IRGenerator::getOperand(Expression::ExpressionUnit *expressionUnit) {
  if (std::holds_alternative<Types::IntegerLiteral>(expressionUnit->operand)) {
    auto integerLiteral =
        std::get<Types::IntegerLiteral>(expressionUnit->operand);

    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(module.getContext()),
                                  stoi(integerLiteral.value));
  }
  // TODO Add the rest
  return nullptr;
}

// TODO: Only handle integer subtraction and addition for now, add more
// operations after semantic parsing is implemented.
llvm::Value *IRGenerator::handle(Expression::Expression &AST) {
  if (builder == nullptr) {
    return nullptr;
  }

  llvm::Value *totalValue = nullptr;
  bool firstExpressionUnit = true;
  for (auto expressionUnit : AST.ExpressionUnits) {

    // Check operand being held.
    llvm::Value *operand = getOperand(expressionUnit);

    if (firstExpressionUnit) {
      firstExpressionUnit = false;
      totalValue = operand;
      continue;
    }
    if (std::holds_alternative<Expression::ArithmeticOperator>(
            *expressionUnit->operation)) {
      auto arithmeticOperator =
          std::get<Expression::ArithmeticOperator>(*expressionUnit->operation);

      switch (arithmeticOperator) {
      case Expression::ArithmeticOperator::PLUS: {
        totalValue = builder->CreateAdd(totalValue, operand);
        break;
      }

      case Expression::ArithmeticOperator::MINUS: {
        totalValue = builder->CreateSub(totalValue, operand);
        break;
      }
      }
    }
  }

  return totalValue;
}

} // namespace IR