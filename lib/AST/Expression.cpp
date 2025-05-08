#include <AST/Expression.h>

// llvm
#include <llvm/IR/Constants.h>
#include <variant>

namespace AST::Expression {

llvm::Value *Expression::codegen(codeGenItems& items) {

  // for (auto expressionUnit : ExpressionUnits) {

  //   llvm::Value *operand = nullptr;

  //   // Check operand being held.
  //   if (std::holds_alternative<Types::IntegerLiteral>(
  //           expressionUnit->operand)) {
  //     auto integerLiteral =
  //         std::get<Types::IntegerLiteral>(expressionUnit->operand);

  //     operand =
  //         llvm::ConstantInt::get(llvm::Type::getInt32Ty(module.getContext()),
  //                                stoi(integerLiteral.value));
  //   } else {
  //     // Add the rest
  //     return nullptr;
  //   }

  //   if (!expressionUnit->operation) {
  //     // end of expression.
  //   }

  //   if (std::holds_alternative<ArithmeticOperator>(
  //           *expressionUnit->operation)) {
  //     auto arithmeticOperator =
  //             std::get<ArithmeticOperator>(*expressionUnit->operation);

  //     switch(arithmeticOperator)
  //     {
  //     case ArithmeticOperator::PLUS:
  //     {
        
  //       break;
  //     }
        
  //     case ArithmeticOperator::MINUS:
  //     {
  //       break;
  //     }
  //     }
  //   }

  //   // switch (expressionUnit->operation) {

  //   // }
  // }

  return llvm::ConstantInt::get(llvm::Type::getInt32Ty(items.module.getContext()),
                                42);
}

} // namespace AST::Expression