#include <IR/IRGenerator.h>

// llvm
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/Casting.h>

namespace IR {

llvm::Value *IRGenerator::handle(Function::FunctionCall &AST) {
  if (builder == nullptr) {
    return nullptr;
  }

  auto callingFunction =
      scopeHandler.getGlobal().getFunctionDeclaration(AST.name);

  if (!callingFunction.has_value()) {
    return nullptr;
  }

  std::vector<llvm::Value *> evaluatedParameters;
  for (auto paramter : AST.parameters) {
    evaluatedParameters.push_back(handle(*paramter));
  }

  return builder->CreateCall(*callingFunction, evaluatedParameters);
}

llvm::Value *IRGenerator::handle(Function::Block &AST) {
  scopeHandler.Push();

  auto statementResult = ASTTraversal::handle(*AST.statement);

  // Expect all generated statements to be non null. If statementResult is
  // empty, it either means that the block only contains a return or its empty
  // (valid case for function declaration).
  for (auto statement : statementResult) {
    if (statement == nullptr) {
      return nullptr;
    }
  }

  // If the returnValue is nullptr, then it means this block has no return. This
  // is ok here. If it is a controlflow block, it will also be handled
  // correctly by the caller. Now just return the latest statement pointer.
  if (AST.returnValue == nullptr) {
    return statementResult.back();
  }

  auto value = handle(*AST.returnValue);
  if (value == nullptr) {
    return nullptr;
  }

  auto *returnValue = builder->CreateRet(value);

  scopeHandler.Pop();

  return returnValue;
}

llvm::Value *IRGenerator::handle(Function::FunctionBody &AST) {

  // TODO Generate directive

  for (auto &parameter : currentFunction->args()) {
    scopeHandler.AddParametersForNextPushedLocalScope(parameter.getName().str(),
                                                      &parameter);
  }

  return AST.block == nullptr ? nullptr : handle(*AST.block);
}

llvm::Value *IRGenerator::handle(Function::Function &AST) {

  std::vector<llvm::Type *> parameterTypes;
  for (auto parameter : AST.parameters) {
    auto type = GetType(parameter->type);
    if (type == nullptr) {
      return nullptr;
    }
    parameterTypes.emplace_back(type);
  }

  auto type = GetType(AST.type);
  if (type == nullptr) {
    return nullptr;
  }

  auto *functionType = llvm::FunctionType::get(type, parameterTypes, false);

  auto function = llvm::Function::Create(
      functionType, llvm::GlobalValue::ExternalLinkage, AST.name, &module);

  unsigned Idx = 0;
  for (auto &parameter : function->args()) {
    auto name = AST.parameters.at(Idx++)->name;
    parameter.setName(name);
  }

  // Save reference to function so that it can be called later.
  scopeHandler.getGlobal().addFunctionDeclaration(AST.name, function);

  // If the function has no body, it is a declaration.
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