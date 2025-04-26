#include <AST/Declaration.h>

// llvm
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Type.h>

// stl
#include <vector>

using namespace AST::Declaration;

llvm::Value *VariableDeclaration::codegen(llvm::Module &module) {
  return new llvm::GlobalVariable(
      module, Declaration::type, false, llvm::GlobalValue::PrivateLinkage,
      nullptr, Declaration::name, nullptr,
      llvm::GlobalValue::ThreadLocalMode::NotThreadLocal, std::nullopt, false);
}

llvm::Value *FunctionDeclaration::codegen(llvm::Module &module) {

  std::vector<llvm::Type *> parameterTypes;
  for (auto parameter : parameters) {
    parameterTypes.emplace_back(parameter->type);
  }

  auto *functionType =
      llvm::FunctionType::get(Declaration::type, parameterTypes, false);

  auto function =
      llvm::Function::Create(functionType, llvm::GlobalValue::PrivateLinkage,
                             Declaration::name, &module);

  unsigned Idx = 0;
  for (auto &parameter : function->args()) {
    parameter.setName(parameters.at(Idx++)->name);
  }
  return function;
}
