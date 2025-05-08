#include <AST/Function.h>
#include <AST/Statement.h>
#include <llvm-19/llvm/IR/BasicBlock.h>
#include <llvm-19/llvm/Support/Casting.h>

namespace AST::Function {

llvm::Value *FunctionCall::codegen(codeGenItems &items) { return nullptr; }

llvm::Value *FunctionBody::codegen(codeGenItems &items) {
  return statement == nullptr ? nullptr : statement->codegen(items);
}

llvm::Value *Function::codegen(codeGenItems &items) {

  std::vector<llvm::Type *> parameterTypes;
  for (auto parameter : parameters) {
    parameterTypes.emplace_back(parameter->type);
  }

  auto *functionType = llvm::FunctionType::get(type, parameterTypes, false);

  auto function = llvm::Function::Create(
      functionType, llvm::GlobalValue::PrivateLinkage, name, &items.module);

  unsigned Idx = 0;
  for (auto &parameter : function->args()) {
    parameter.setName(parameters.at(Idx++)->name);
  }

  if (body != nullptr) {
    items.currentFunction = function;
    if (body->codegen(items) == nullptr) {
      return nullptr;
    }
  }
  items.currentFunction = nullptr;
  return function;
}
} // namespace AST::Function