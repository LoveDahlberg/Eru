#pragma once

#include "Support/Scope.h"
#include <AST/ASTContext.h>
#include <AST/Assignment.h>
#include <Analyzer/AnalyzerTypes.h>
#include <Support/Result.h>

#include <Analyzer/Expression.h>
#include <Analyzer/Function.h>
#include <Analyzer/Variable.h>

namespace Analyzer {

struct PublicAnalyzer {
  virtual ExpressionAnalyzer &expression() = 0;
  virtual FunctionAnalyzer &function() = 0;
  virtual VariableAnalyzer &variable() = 0;

  virtual void PrepareFunctionScope(AnalyzerScopeContextData *contextData) = 0;

  virtual void PushScope(scopeKind kind) = 0;
  virtual void PopScope() = 0;
};

struct PrivateAnalyzer : PublicAnalyzer {
  virtual AST::Context::ASTContext &getASTContext() = 0;

  virtual AnalyzerScope &getCurrentScope() = 0;
  virtual bool currentScopeIsGlobal() = 0;

  virtual AnalyzerGlobalScope &getGlobalScope() = 0;
  virtual AnalyzerFunctionScope *getFunction() = 0;
  virtual AnalyzerLocalScope *getLocalScope() = 0;
};

class Analyzer : public PrivateAnalyzer {
  AST::Context::ASTContext &astContext;
  AnalyzerScopeHandler scopeHandler;

  ExpressionAnalyzer expressionHandle;
  FunctionAnalyzer functionHandle;
  VariableAnalyzer variableHandle;

public:
  Analyzer() = delete;

  Analyzer(AST::Context::ASTContext &astContext)
      : astContext(astContext), expressionHandle(*this), functionHandle(*this),
        variableHandle(*this) {}

  // -- Implementation of PublicAnalyzer
  ExpressionAnalyzer &expression() { return expressionHandle; }

  FunctionAnalyzer &function() { return functionHandle; }

  VariableAnalyzer &variable() { return variableHandle; }

  // -- Implementation of PrivateAnalyzer
  AST::Context::ASTContext &getASTContext() { return astContext; }

  AnalyzerScope &getCurrentScope() { return scopeHandler.getCurrent(); }

  bool currentScopeIsGlobal() {
    return getCurrentScope().getScopeKind() ==
           Support::Scope::scopeKind::GLOBAL;
  }

  AnalyzerGlobalScope &getGlobalScope() { return scopeHandler.getGlobal(); }

  AnalyzerLocalScope *getLocalScope() {
    return scopeHandler.CastCurrentToLocalScope();
  }
  AnalyzerFunctionScope *getFunction() {
    return scopeHandler.findFunctionFrom(&getCurrentScope());
  }
  // --

  void PushScope(scopeKind kind) { scopeHandler.Push(kind); }

  void PopScope() { scopeHandler.Pop(); }

  void PrepareFunctionScope(AnalyzerScopeContextData *contextData) {
    scopeHandler.PrepareFunctionScope(contextData);
  }
};

} // namespace Analyzer