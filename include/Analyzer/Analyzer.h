#pragma once

#include <AST/ASTContext.h>
#include <AST/Assignment.h>
#include <Support/Result.h>

#include <Analyzer/Expression.h>
#include <Analyzer/Function.h>
#include <Analyzer/Variable.h>

// std
#include <memory>

namespace Analyzer {

struct PublicAnalyzer {
  virtual ExpressionAnalyzer &expression() = 0;
  virtual FunctionAnalyzer &function() = 0;
  virtual VariableAnalyzer &variable() = 0;

  virtual void PushScope() = 0;
  virtual void PopScope() = 0;
};

struct PrivateAnalyzer : PublicAnalyzer {
  virtual AST::Context::ASTContext &getASTContext() = 0;
  virtual std::unique_ptr<Scope> &getCurrentScope() = 0;
};

struct Scope {

  Scope(bool isGlobal = false) : isGlobal(isGlobal) {}

  std::unique_ptr<Scope> parentScope;
  std::vector<AST::VariableDeclaration::Variable *> variableDeclarations;

  bool isGlobal;
};

class Analyzer : public PrivateAnalyzer {
  AST::Context::ASTContext &astContext;
  std::unique_ptr<Scope> currentScope;

  ExpressionAnalyzer expressionHandle;
  FunctionAnalyzer functionHandle;
  VariableAnalyzer variableHandle;

public:
  Analyzer() = delete;

  Analyzer(AST::Context::ASTContext &astContext)
      : astContext(astContext), currentScope(std::make_unique<Scope>(true)),
        expressionHandle(*this), functionHandle(*this), variableHandle(*this) {}

  // Implementation of PublicAnalyzer
  ExpressionAnalyzer &expression() { return expressionHandle; }
  FunctionAnalyzer &function() { return functionHandle; }
  VariableAnalyzer &variable() { return variableHandle; }

  /// Call when going into a new scope.
  void PushScope() {

    // New scope is always a local scope.
    auto newChildScope = std::make_unique<Scope>(false);

    newChildScope->parentScope = std::move(currentScope);
    currentScope = std::move(newChildScope);
  }

  /// Call when going out of the current scope.
  void PopScope() {
    // Drop the current scope. Set current scope to the previous parent scope.
    currentScope = std::move(currentScope->parentScope);
  }

  // Implementation of PrivateAnalyzer
  AST::Context::ASTContext &getASTContext() { return astContext; }
  std::unique_ptr<Scope> &getCurrentScope() { return currentScope; }

};

} // namespace Analyzer