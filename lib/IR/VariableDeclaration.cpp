#include <IR/IRGenerator.h>
#include <Support/Result.h>

// llvm
#include <llvm/IR/Constant.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>

namespace IR {

Result<llvm::Value *>
IRGenerator::handle(VariableDeclaration::VariableDeclaration &AST) {

  const auto &astType = AST.variable->type;

  auto type = GetType(astType);

  RET_ON_TRUE(!type.isSuccessful() || builder == nullptr,
              "IRGenerator VariableDeclaration: type or builder null");

  auto *variable =
      builder->CreateAlloca(astType.isPointer ? builder->getPtrTy() : *type,
                            nullptr, AST.variable->name);

  scopeHandler.getCurrent().addVariableDeclaration(
      AST.variable->name,
      new ScopeVariable{variable, *type, true, astType.pointerDepth});

  return variable;
}

Result<llvm::Value *>
IRGenerator::handle(VariableDeclaration::GlobalVariableInitialization &AST) {

  auto *variableDeclaration = AST.variableDeclaration->variable;
  const auto &astType = variableDeclaration->type;

  auto type = GetType(variableDeclaration->type);
  RET_ON_FAILURE(
      type, "IRGenerator: GlobalVariableInitialization: Error to get type.");

  llvm::Constant *operand = nullptr;

  if (AST.constOperand.has_value()) {
    if (std::holds_alternative<Types::IntegerLiteral>(*AST.constOperand)) {
      auto integerLiteral = std::get<Types::IntegerLiteral>(*AST.constOperand);

      operand =
          llvm::ConstantInt::get(llvm::Type::getInt32Ty(module.getContext()),
                                 stoi(integerLiteral.value));
    } else {
      llvm::report_fatal_error(
          "IR Global variable initialization: encountered unimplemented type");
    }
  }

  auto variable = new llvm::GlobalVariable(
      module, astType.isPointer ? builder->getPtrTy() : *type, false,
      llvm::GlobalValue::ExternalLinkage, operand, variableDeclaration->name,
      nullptr, llvm::GlobalValue::ThreadLocalMode::NotThreadLocal, std::nullopt,
      false);

  scopeHandler.getGlobal().addVariableDeclaration(
      variableDeclaration->name,
      new ScopeVariable{variable, *type, true, astType.pointerDepth});

  return variable;
}

} // namespace IR