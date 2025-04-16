#include <AST/Declaration.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>

#include <iostream>

using namespace AST::Declaration;

llvm::Value *VariableDeclaration::codegen(llvm::Module &module) {
  // auto asd = new llvm::GlobalVariable(
  //     module, Declaration::type, false, llvm::GlobalValue::PrivateLinkage,
  //     nullptr, Declaration::name, nullptr,
  //     llvm::GlobalValue::ThreadLocalMode::NotThreadLocal, std::nullopt, false);
  llvm::Constant *stringConstant;
  auto asd = new llvm::GlobalVariable(module,
    Declaration::type,
    true,
    llvm::GlobalValue::PrivateLinkage,
    stringConstant,
    "");

  std::cout << asd->getValueName();
}

llvm::Value *FunctionDeclaration::codegen(llvm::Module &module) {
  return nullptr;
}
