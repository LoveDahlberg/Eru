#pragma once

#include <AST/ASTContext.h>
#include <Support/Result.h>

// std
#include <memory>

namespace Analyzer {

struct Scope {

  Scope(bool isGlobal = false) : isGlobal(isGlobal) {}

  std::unique_ptr<Scope> parentScope;
  std::vector<AST::VariableDeclaration::Variable *> variableDeclarations;

  bool isGlobal;
};

class Analyzer {
  AST::Context::ASTContext &astContext;
  std::unique_ptr<Scope> currentScope;
  std::vector<AST::Function::Function *> functions;

  bool isOkToDeclareVariable(AST::VariableDeclaration::Variable *variable,
                          std::unique_ptr<Scope> &scope);
  bool
  isDeclaredVariableParentScope(AST::VariableDeclaration::Variable *variable);
  bool addVariableDeclaration(AST::VariableDeclaration::Variable *variable);

  /// Attempt to add a function. Returns an error if not successful.
  Result<bool> addFunction(AST::Function::Function *function, AST::Function::FunctionStatus status);

public:

  Analyzer() = delete;
  
  Analyzer(AST::Context::ASTContext &astContext)
      : astContext(astContext), currentScope(std::make_unique<Scope>(true)) {}

  /// Called when going into a new scope.
  void PushScope() {

    // New scope is always a local scope.
    auto newChildScope = std::make_unique<Scope>(false);

    newChildScope->parentScope = std::move(currentScope);
    currentScope = std::move(newChildScope);
  }

  /// Called when going out of the current scope.
  void PopScope() {
    // Drop the current scope. Set current scope to the previous parent scope.
    currentScope = std::move(currentScope->parentScope);
  }

  // Act on methods
  Result<bool>
  ActOnVariableDeclaration(AST::VariableDeclaration::Variable *variable);

  Result<bool> ActOnFunctionDeclaration(AST::Function::Function *function);
  Result<bool> ActOnFunctionDefinition(AST::Function::Function *function);
  
  Result<bool> ActOnFunctionCall(AST::Function::FunctionCall* call);
};

} // namespace Analyzer