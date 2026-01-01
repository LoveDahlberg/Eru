#include "Support/Result.h"
#include "Support/Scope.h"
#include <IR/IRGenerator.h>

// llvm
#include <llvm/IR/Value.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/LLVMContext.h>
#include <llvm/Support/Casting.h>

namespace IR {

Result<llvm::Value *> IRGenerator::handle(Function::FunctionCall &AST) {
  RET_ON_TRUE(builder == nullptr, "IRGenerator: FunctionCall: builder is null");

  auto callingFunction =
      scopeHandler.getGlobal().getFunctionDeclaration(AST.name);

  if (!callingFunction.has_value()) {
    return nullptr;
  }

  std::vector<llvm::Value *> evaluatedParameters;
  for (auto paramter : AST.parameters) {
    auto maybeParameter = handle(*paramter);
    RET_ON_FAILURE(maybeParameter,
                   "IRGenerator: FunctionCall: builder is null");

    evaluatedParameters.push_back(*maybeParameter);
  }

  return builder->CreateCall(callingFunction->function, evaluatedParameters);
}

Result<llvm::Value *> IRGenerator::handle(Function::Block &AST) {
  scopeHandler.Push(AST.scopeKind);

  // Declare parameters as variables in current scope, if kind is function.
  if (AST.scopeKind == Support::Scope::scopeKind::FUNCTION) {
    auto *localScope = scopeHandler.CastCurrentToLocalScope();

    if (localScope == nullptr) {
      scopeHandler.Pop();
      return {nullptr, "IRGenerator: Function::Block: no local scope"};
    }
    for (auto &[name, paramter] : localScope->getContextData()->parameters) {
      localScope->addVariableDeclaration(name, &paramter);
    }
  }

  auto statementResult = ASTTraversal::handle(*AST.statement);

  // Expect all generated statements to be non null. If statementResult is
  // empty, it either means that the block only contains a return or its empty
  // (valid case for function declaration).
  for (auto statement : statementResult) {
    if (!statement.isSuccessful()) {
      scopeHandler.Pop();
      return {nullptr, "IRGenerator: Function::Block: statement is null"};
    }
  }

  // If the returnValue is nullptr, then it means this block has no return. This
  // is ok here. If it is a controlflow block, it will also be handled
  // correctly by the caller. Now just return the latest statement pointer.
  if (AST.returnValue == nullptr) {
    scopeHandler.Pop();
    return statementResult.empty() ? nullptr : statementResult.back();
  }

  auto value = handle(*AST.returnValue);
  if (!value.isSuccessful()) {
    scopeHandler.Pop();
    return {nullptr, "IRGenerator: Function::Block: return value null"};
  }

  auto *returnValue = builder->CreateRet(*value);

  scopeHandler.Pop();

  return returnValue;
}

Result<llvm::Value *> IRGenerator::handle(Function::FunctionBody &AST) {

  // TODO Generate directive

  auto maybeScopefunction =
      scopeHandler.getGlobal().getFunctionDeclaration(AST.functionName);

  RET_ON_TRUE(!maybeScopefunction.has_value() || AST.block == nullptr,
              "IRGenerator: FunctionBody: block is null "
              "or function cannot be found is null");

  auto scopeFunction = *maybeScopefunction;

  auto &context = module.getContext();
  auto *basicBlock =
      llvm::BasicBlock::Create(context, "block", scopeFunction.function);
  if (builder == nullptr) {
    builder = new llvm::IRBuilder<llvm::NoFolder>(context);
  }
  builder->SetInsertPoint(basicBlock);

  unsigned Idx = 0;
  IRScopeContextData contextData;
  for (auto &parameter : scopeFunction.function->args()) {

    const auto &astType = scopeFunction.underlyingParameterTypes.at(Idx++);

    auto type = GetType(astType);
    RET_ON_FAILURE(type,
                   "IRGenerator: FunctionBody: Error to get parameter type.");

    contextData.parameters.try_emplace(parameter.getName().str(), &parameter,
                                       *type, false, astType.pointerDepth);
  }

  contextData.currentFunction = scopeFunction.function;

  scopeHandler.PrepareFunctionScope(&contextData);

  return handle(*AST.block);
}

Result<llvm::Value *> IRGenerator::handle(Function::FunctionDeclaration &AST) {

  IR::ScopeFunction scopeFunction;

  std::vector<llvm::Type *> parameterTypes;
  for (auto parameter : AST.parameters) {

    // TODO: make this less confusing. I just need a way to pass the Types:Type
    // as well as the underlying type to the FunctionBody handling function.
    // Right now I save the entire thing and just get the type again, which is
    // not very efficient and it is very confusing.
    scopeFunction.underlyingParameterTypes.emplace_back(parameter->type);

    auto type = GetType(parameter->type);
    RET_ON_FAILURE(
        type, "IRGenerator: FunctionDeclaration: Error to get parameter type.");

    parameterTypes.emplace_back(
        parameter->type.isPointer
            ? llvm::PointerType::get(module.getContext(), 0)
            : *type);
  }

  auto type = GetType(AST.type);
  RET_ON_FAILURE(type,
                 "IRGenerator: FunctionDeclaration: Error to get return type.");

  auto *functionType = llvm::FunctionType::get(
      AST.type.isPointer ? llvm::PointerType::get(module.getContext(), 0)
                         : *type,
      parameterTypes, false);

  auto function = llvm::Function::Create(
      functionType, llvm::GlobalValue::ExternalLinkage, AST.name, &module);

  unsigned Idx = 0;
  for (auto &parameter : function->args()) {
    auto name = AST.parameters.at(Idx++)->name;
    parameter.setName(name);
  }

  scopeFunction.function = function;

  // Save reference to function so that it can be called later.
  scopeHandler.getGlobal().addFunctionDeclaration(AST.name, scopeFunction);

  return function;
}
} // namespace IR