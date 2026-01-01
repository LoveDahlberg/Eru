#include <IR/IRGenerator.h>
#include <llvm/IR/DerivedTypes.h>
#include <llvm/IR/Type.h>

namespace IR {

Result<llvm::Type *> IRGenerator::GetType(Types::Type type) {

  auto &context = module.getContext();

  switch (type.dataType) {
  case Types::DataType::INT:
    return llvm::Type::getInt32Ty(context);
  case Types::DataType::SINT32:
    return llvm::Type::getInt32Ty(context);
  case Types::DataType::UINT32:
    return llvm::Type::getInt32Ty(context);
  case Types::DataType::BOOl:
    return llvm::Type::getInt1Ty(context);
    break;
  case Types::DataType::CHAR:
    return llvm::Type::getInt8Ty(context);
  case Types::DataType::STRING:
    // TODO implement string handling
    return llvm::StructType::create(context, "string");
  default:
    // err
    return {nullptr, "IRGenerator: GetType: invalid type."};
  }
}
} // namespace IR