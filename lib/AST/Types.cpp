// Include
#include <AST/Types.h>

// llvm
#include <llvm/IR/Type.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Constants.h>

namespace AST::Types {

llvm::Value* Type::codegen() {
  return nullptr;
}


llvm::Value* Int::codegen() {
  auto ctx = llvm::LLVMContext();
  auto type = llvm::Type::getInt32Ty(ctx);
  auto contvalue = llvm::ConstantInt::get(type, 1);
  return contvalue;
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