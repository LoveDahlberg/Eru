#include <AST/Function.h>

namespace AST::Function {

llvm::Value *FunctionBody::codegen(llvm::Module &module) { return nullptr; }

llvm::Value *FunctionCall::codegen(llvm::Module &module) { return nullptr; }

llvm::Value *Function::codegen(llvm::Module &module) { return nullptr; }
} // namespace AST::Function