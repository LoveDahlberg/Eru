#include <IR/IRGenerator.h>

// llvm
#include <llvm-20/llvm/Support/Error.h>
#include <llvm/IR/Constants.h>

namespace IR {

llvm::Value *
IRGenerator::getOperand(Expression::ExpressionUnit *expressionUnit) {
  if (std::holds_alternative<Types::IntegerLiteral>(expressionUnit->operand)) {
    auto integerLiteral =
        std::get<Types::IntegerLiteral>(expressionUnit->operand);

    return llvm::ConstantInt::get(llvm::Type::getInt32Ty(module.getContext()),
                                  stoi(integerLiteral.value));
  } else if (std::holds_alternative<Function::FunctionCall *>(
                 expressionUnit->operand)) {
    auto call = std::get<Function::FunctionCall *>(expressionUnit->operand);

    return handle(*call);

  } else if (std::holds_alternative<Types::NamedIdentifier>(
                 expressionUnit->operand)) {
    auto identifier = std::get<Types::NamedIdentifier>(expressionUnit->operand);

    // Make sure the variable value being pointed to is used here, as the
    // expression is done on the value itself.
    // TODO: If pointer arithmetic is added, extend this logic to handle it.
    auto result =
        scopeHandler.getCurrent().getDeclaredVariable(identifier.value);

    return result.has_value() ? result->getValue(builder) : nullptr;

  } else {
    llvm::report_fatal_error(
        "IRExpression: getOperand: encountered unimplemented type");
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