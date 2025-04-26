#include <AST/Function.h>

namespace AST::Function {

llvm::Value *FunctionDefinition::codegen(llvm::Module &module) {
  return nullptr;
}

llvm::Value *FunctionCall::codegen(llvm::Module &module) { return nullptr; }
} // namespace AST::Function