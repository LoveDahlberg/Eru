#include <IR/IRGenerator.h>

namespace IR {

Result<llvm::Type *> IRGenerator::GetType(Types::Type type) {
  switch (type.dataType) {
  case Types::DataType::INT:
    return llvm::Type::getInt32Ty(module.getContext());
  case Types::DataType::SINT32:
    return llvm::Type::getInt32Ty(module.getContext());
  case Types::DataType::UINT32:
    return llvm::Type::getInt32Ty(module.getContext());
  case Types::DataType::BOOl:
    return llvm::Type::getInt1Ty(module.getContext());
    break;
  case Types::DataType::CHAR:
    return llvm::Type::getInt8Ty(module.getContext());
  case Types::DataType::STRING:
    // TODO implement string handling
    return llvm::StructType::create(module.getContext(), "string");
  default:
    // err
    return {nullptr, "IRGenerator: GetType: invalid type."};
  }
}
} // namespace IR