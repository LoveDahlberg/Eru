#include <IR/IRGenerator.h>

namespace IR {

llvm::Type *IRGenerator::GetType(Types::Types type) {
  switch (type) {
  case Types::Types::INT:
    return llvm::Type::getInt32Ty(module.getContext());
  case Types::Types::SINT32:
    return llvm::Type::getInt32Ty(module.getContext());
  case Types::Types::UINT32:
    return llvm::Type::getInt32Ty(module.getContext());
  case Types::Types::BOOl:
    return llvm::Type::getInt1Ty(module.getContext());
    break;
  case Types::Types::CHAR:
    return llvm::Type::getInt8Ty(module.getContext());
  case Types::Types::STRING:
    // TODO implement string handling
    return llvm::StructType::create(module.getContext(), "string");
  default:
    // err
    return nullptr;
  }
}
} // namespace IR