#include <AST/Function.h>
#include <AST/Statement.h>
#include <llvm-19/llvm/IR/BasicBlock.h>
#include <llvm-19/llvm/Support/Casting.h>

namespace AST::Function {

llvm::Value *FunctionCall::codegen(llvm::Module &module) { return nullptr; }

llvm::Value *FunctionBody::codegen(llvm::Module &module) {
  return statement == nullptr ? nullptr : statement->codegen(module);
}

llvm::Value *Function::codegen(llvm::Module &module) {

  std::vector<llvm::Type *> parameterTypes;
  for (auto parameter : parameters) {
    parameterTypes.emplace_back(parameter->type);
  }

  auto *functionType = llvm::FunctionType::get(type, parameterTypes, false);

  auto function = llvm::Function::Create(
      functionType, llvm::GlobalValue::PrivateLinkage, name, &module);

  unsigned Idx = 0;
  for (auto &parameter : function->args()) {
    parameter.setName(parameters.at(Idx++)->name);
  }

  if (body != nullptr) {
    auto *start = body->codegen(module);
    if (auto block = llvm::dyn_cast<llvm::BasicBlock>(start)) {
      function->insert(function->getIterator()->begin(), block);
    }
  }

  return function;
}
} // namespace AST::Function