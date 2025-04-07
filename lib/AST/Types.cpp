#include <AST/Types.h>

namespace AST::Types {



llvm::Value* Type::codegen() {
  return nullptr;
}


llvm::Value* Int::codegen() {
  return nullptr;
}


llvm::Value* SingedInt32::codegen() {
  return nullptr;
}


llvm::Value* UnsignedInt32::codegen() {
  return nullptr;
}


llvm::Value* Bool::codegen() {
  return nullptr;
}


llvm::Value* String::codegen() {
  return nullptr;
}


llvm::Value* Char::codegen() {
  return nullptr;
}


llvm::Value* Identifier::codegen() {
  return nullptr;
}

}