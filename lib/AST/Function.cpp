#include <AST/Function.h>
#include <AST/Statement.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/Support/Casting.h>
#include <llvm/IR/Type.h>

namespace AST::Function {

llvm::Value *FunctionCall::codegen(codeGenItems &items) {
  if (items.builder == nullptr) {
    return nullptr;
  }

  // TODO make use of symbol table here?
  auto callingFunction = items.module.getFunction(name);
  if (callingFunction == nullptr) {
    return nullptr;
  }

  std::vector<llvm::Value *> evaluatedParameters;
  for (auto paramter : parameters) {
    evaluatedParameters.push_back(paramter->codegen(items));
  }

  return items.builder->CreateCall(callingFunction, evaluatedParameters);
}

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
      functionType, llvm::GlobalValue::ExternalLinkage, name, &items.module);

  unsigned Idx = 0;
  for (auto &parameter : function->args()) {
    parameter.setName(parameters.at(Idx++)->name);
  }

  if (body != nullptr) {
    auto &context = items.module.getContext();
    auto *basicBlock = llvm::BasicBlock::Create(context, "statement", function);
    if (items.builder == nullptr) {
      items.builder = new llvm::IRBuilder<llvm::NoFolder>(context);
    }
    items.builder->SetInsertPoint(basicBlock);

    items.currentFunction = function;
    if (body->codegen(items) == nullptr) {
      return nullptr;
    }

    if (type->isIntegerTy(32)) {
      items.builder->CreateRet(llvm::ConstantInt::get(
          type, 0));
    } else {
      return nullptr;
    }
  }

  items.currentFunction = nullptr;
  return function;
}
} // namespace AST::Function