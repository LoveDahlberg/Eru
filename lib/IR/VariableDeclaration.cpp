#include <IR/IRGenerator.h>

// llvm
#include <llvm/IR/Function.h>
#include <llvm/IR/GlobalValue.h>
#include <llvm/IR/GlobalVariable.h>
#include <llvm/IR/Instructions.h>
#include <llvm/IR/Type.h>

namespace IR {

llvm::Value *
IRGenerator::handle(VariableDeclaration::VariableDeclaration &AST) {

  auto *type = GetType(AST.variable->type);
  if (type == nullptr) {
    return nullptr;
  }

  auto storeDeclaration = [&AST, &type, this](auto *variable) {
    ;
    scopeHandler.getCurrent().variableDeclarations.emplace(
        AST.variable->name, ScopeVariable{variable, type});
    return variable;
  };

  if (AST.isGlobal) {
    return storeDeclaration(new llvm::GlobalVariable(
        module, type, false, llvm::GlobalValue::ExternalLinkage, nullptr,
        AST.variable->name, nullptr,
        llvm::GlobalValue::ThreadLocalMode::NotThreadLocal, std::nullopt,
        false));
  }

  return builder != nullptr ? storeDeclaration(builder->CreateAlloca(
                                  type, nullptr, AST.variable->name))
                            : nullptr;
}

} // namespace IR