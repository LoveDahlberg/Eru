#include <AST/VariableDeclaration.h>

// llvm
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>

// stl
#include <vector>

using namespace AST::VariableDeclaration;

llvm::Value *VariableDeclaration::codegen(llvm::Module &module) {
  if (global) {
    return new llvm::GlobalVariable(
        module, variable->type, false, llvm::GlobalValue::PrivateLinkage,
        nullptr, variable->name, nullptr,
        llvm::GlobalValue::ThreadLocalMode::NotThreadLocal, std::nullopt,
        false);
  }
  return new llvm::AllocaInst(variable->type, 0, nullptr, {}, variable->name,
                              nullptr);
}