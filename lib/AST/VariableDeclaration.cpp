#include <AST/VariableDeclaration.h>

// llvm
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>

using namespace AST::VariableDeclaration;

llvm::Value *VariableDeclaration::codegen(codeGenItems &items) {
  if (global) {
    return new llvm::GlobalVariable(
        items.module, variable->type, false, llvm::GlobalValue::PrivateLinkage,
        nullptr, variable->name, nullptr,
        llvm::GlobalValue::ThreadLocalMode::NotThreadLocal, std::nullopt,
        false);
  }

  if (items.builder == nullptr) {
    return nullptr;
  }

  return items.builder->CreateAlloca(variable->type, nullptr, variable->name);
}