#include <IR/IRGenerator.h>

// llvm
#include <llvm/IR/Constant.h>
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>

namespace IR {

llvm::Value *
IRGenerator::handle(VariableDeclaration::VariableDeclaration &AST) {

  auto *type = GetType(AST.variable->type);
  if (type == nullptr || builder == nullptr) {
    return nullptr;
  }

  auto *variable = builder->CreateAlloca(type, nullptr, AST.variable->name);

  scopeHandler.getCurrent().addVariableDeclaration(
      AST.variable->name, ScopeVariable{variable, type});

  return variable;
}

llvm::Value *
IRGenerator::handle(VariableDeclaration::GlobalVariableInitialization &AST) {

  auto *variableDeclaration = AST.variableDeclaration->variable;

  auto *type = GetType(variableDeclaration->type);

  if (type == nullptr) {
    return nullptr;
  }

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
      module, type, false, llvm::GlobalValue::ExternalLinkage, operand,
      variableDeclaration->name, nullptr,
      llvm::GlobalValue::ThreadLocalMode::NotThreadLocal, std::nullopt, false);

  scopeHandler.getGlobal().addVariableDeclaration(
      variableDeclaration->name, ScopeVariable{variable, type});

  return variable;
}

} // namespace IR