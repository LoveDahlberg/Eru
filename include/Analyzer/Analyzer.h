#pragma once

#include <AST/ASTContext.h>

// std
#include <memory>

namespace Analyzer {

struct Scope {

  Scope(bool isGlobal = false) : isGlobal(isGlobal) {}

  std::unique_ptr<Scope> ParentScope;
  std::vector<AST::VariableDeclaration::Variable *> VariableDeclarations;

  bool isGlobal;
};

class Analyzer {
  AST::Context::ASTContext &astContext;
  std::unique_ptr<Scope> CurrentScope;

  bool
  isDeclaredVariableParentScope(AST::VariableDeclaration::Variable *variable);
  bool
  declareVariableCurrentScope(AST::VariableDeclaration::Variable *variable);

public:
  Analyzer(AST::Context::ASTContext &astContext)
      : astContext(astContext), CurrentScope(std::make_unique<Scope>(true)) {}

  /// Called when going into a new scope.
  void PushScope() {

    // New scope is always a local scope.
    auto newChildScope = std::make_unique<Scope>(false);

    newChildScope->ParentScope = std::move(CurrentScope);
    CurrentScope = std::move(newChildScope);
  }

  /// Called when going out of the current scope.
  void PopScope() {
    // Drop the current scope. Set current scope to the previous parent scope.
    CurrentScope = std::move(CurrentScope->ParentScope);
  }

  // Act on methods
  void ActOnVariableDeclaration(AST::VariableDeclaration::Variable *variable);
  
  void ActOnFunctionDeclaration(AST::Function::Function* function);
  void ActOnFunctionImplementation(AST::Function::Function* function);
};

} // namespace Analyzer