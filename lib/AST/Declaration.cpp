#include <AST/Declaration.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>

using namespace AST::Declaration;

llvm::Value *VariableDeclaration::codegen(llvm::Module &module) {
  return new llvm::GlobalVariable(Declaration::type, false,
                                      llvm::GlobalValue::PrivateLinkage, {},
                                      Declaration::name);
                                      
}

llvm::Value *FunctionDeclaration::codegen(llvm::Module &module) {
  return nullptr;
}
