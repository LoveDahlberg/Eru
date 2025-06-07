#include <IR/IRGenerator.h>

// llvm
#include <llvm/IR/BasicBlock.h>
#include <llvm/Support/Casting.h>

namespace IR {

llvm::Value *IRGenerator::handle(Function::FunctionCall &AST) {
  if (builder == nullptr) {
    return nullptr;
  }

  // TODO make use of symbol table here?
  auto callingFunction = module.getFunction(AST.name);
  if (callingFunction == nullptr) {
    return nullptr;
  }

  std::vector<llvm::Value *> evaluatedParameters;
  for (auto paramter : AST.parameters) {
    evaluatedParameters.push_back(handle(*paramter));
  }

  return builder->CreateCall(callingFunction, evaluatedParameters);
}

llvm::Value *IRGenerator::handle(Function::Block &AST) {
  auto statementResult = ASTTraversal::handle(*AST.statement);

  // If the returnValue is nullptr, then it means this block has no return (this
  // is ok). Note that this block is not an IR basic block, it is the grammar
  // block.
  if (AST.returnValue == nullptr || statementResult.empty()) {
    return nullptr;
  }

  auto value = handle(*AST.returnValue);
  if (value == nullptr) {
    return nullptr;
  }

  return builder->CreateRet(value);
}

llvm::Value *IRGenerator::handle(Function::FunctionBody &AST) {

  // Generate directive

  return AST.block == nullptr ? nullptr : handle(*AST.block);
}

llvm::Value *IRGenerator::handle(Function::Function &AST) {

  std::vector<llvm::Type *> parameterTypes;
  for (auto parameter : AST.parameters) {
    parameterTypes.emplace_back(parameter->type);
  }

  auto *functionType = llvm::FunctionType::get(AST.type, parameterTypes, false);

  auto function = llvm::Function::Create(
      functionType, llvm::GlobalValue::ExternalLinkage, AST.name, &module);

  unsigned Idx = 0;
  for (auto &parameter : function->args()) {
    parameter.setName(AST.parameters.at(Idx++)->name);
  }

  if (AST.body != nullptr) {
    auto &context = module.getContext();
    auto *basicBlock = llvm::BasicBlock::Create(context, "block", function);
    if (builder == nullptr) {
      builder = new llvm::IRBuilder<llvm::NoFolder>(context);
    }
    builder->SetInsertPoint(basicBlock);

    currentFunction = function;
    if (handle(*AST.body) == nullptr) {
      return nullptr;
    }
  }

  currentFunction = nullptr;
  return function;
}
} // namespace IR