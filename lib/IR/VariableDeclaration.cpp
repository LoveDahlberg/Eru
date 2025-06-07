#include <IR/IRGenerator.h>

// llvm
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>

namespace IR {

llvm::Value *
IRGenerator::handle(VariableDeclaration::VariableDeclaration &AST) {
  if (AST.global) {
    return new llvm::GlobalVariable(
        module, AST.variable->type, false, llvm::GlobalValue::ExternalLinkage,
        nullptr, AST.variable->name, nullptr,
        llvm::GlobalValue::ThreadLocalMode::NotThreadLocal, std::nullopt,
        false);
  }

  if (builder == nullptr) {
    return nullptr;
  }

  return builder->CreateAlloca(AST.variable->type, nullptr, AST.variable->name);
}

} // namespace IR