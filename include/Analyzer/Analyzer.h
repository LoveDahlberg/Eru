#pragma once

#include "AST/Assignment.h"
#include "AST/Expression.h"
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


// TODO: Rewrite this so that the top analyzer object holds all other analyzer sub categories.
// Each subcategory type should be able to easily call other sub categories through the analyzer.
// See if Curiously Recurring Template Pattern (CRTP) makes sense.
class Analyzer {
  AST::Context::ASTContext &astContext;
  std::unique_ptr<Scope> currentScope;
  std::vector<AST::Function::Function *> functions;


  Result<bool>
  isTypeCheckedVariableDeclared(AST::VariableDeclaration::Variable *variable,
                                std::unique_ptr<Scope> &scope);
  bool isVariableDeclared(AST::VariableDeclaration::Variable *variable,
                          std::unique_ptr<Scope> &scope);

  AST::VariableDeclaration::Variable *
  getDeclaredVariable(AST::Types::NamedIdentifier *identifier,
                      std::unique_ptr<Scope> &scope);

  bool
  isVariableDeclaredParentScope(AST::VariableDeclaration::Variable *variable);
  Result<bool> addVariableDeclarationToCurrentScope(
      AST::VariableDeclaration::Variable *variable);

  /// Attempt to add a function. Returns an error if not successful.
  Result<bool> addFunction(AST::Function::Function *function,
                           AST::Function::FunctionStatus status);

  Result<AST::VariableDeclaration::VariableDeclaration *>
  declareVariable(AST::VariableDeclaration::Variable *variable);

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
  ActOnGlobalVariableDeclaration(AST::VariableDeclaration::Variable *variable);
  Result<bool>
  ActOnLocalVariableDeclaration(AST::VariableDeclaration::Variable *variable,
                                AST::Statement::Statement *statement);
  Result<bool> ActOnAssignment(AST::Assignment::Assignment *assignment,
                               AST::Statement::Statement *statement);

  Result<bool> ActOnFunctionDeclaration(AST::Function::Function *function);
  Result<bool> ActOnFunctionDefinition(AST::Function::Function *function);
  Result<bool> ActOnFunctionCall(AST::Function::FunctionCall *call);

  Result<bool> ActOnExpression(AST::Expression::Expression *expression,
                               const AST::Types::Types &type);
};

} // namespace Analyzer