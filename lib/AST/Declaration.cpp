#include <AST/Declaration.h>

using namespace AST::Declaration;

llvm::Value *Declaration::codegen() { return nullptr; }

llvm::Value *VariableDeclaration::codegen() { return nullptr; }

llvm::Value *FunctionDeclaration::codegen() { return nullptr; }
