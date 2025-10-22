#pragma once

#include <AST/ASTContext.h>
#include <AST/Assignment.h>
#include <Support/Result.h>
#include <Support/Scope.h>

#include <Analyzer/Expression.h>
#include <Analyzer/Function.h>
#include <Analyzer/Variable.h>

using namespace Support::Scope;

namespace Analyzer {

using AnalyzerScopeHandler = ScopeHandler<AST::VariableDeclaration::Variable *,
                                          AST::Function::Function *>;
using AnalyzerScope = Scope<AST::VariableDeclaration::Variable *>;
using AnalyzerGlobalScope = GlobalScope<AST::VariableDeclaration::Variable *,
                                        AST::Function::Function *>;

struct PublicAnalyzer {
  virtual ExpressionAnalyzer &expression() = 0;
  virtual FunctionAnalyzer &function() = 0;
  virtual VariableAnalyzer &variable() = 0;

  virtual void PushScope() = 0;
  virtual void PopScope() = 0;
};

struct PrivateAnalyzer : PublicAnalyzer {
  virtual AST::Context::ASTContext &getASTContext() = 0;
  virtual AnalyzerScope &getCurrentScope() = 0;
  virtual AnalyzerGlobalScope &getGlobalScope() = 0;
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

  // Implementation of PublicAnalyzer
  ExpressionAnalyzer &expression() { return expressionHandle; }
  FunctionAnalyzer &function() { return functionHandle; }
  VariableAnalyzer &variable() { return variableHandle; }

  void PushScope() { scopeHandler.Push(); }

  void PopScope() { scopeHandler.Pop(); }

  // Implementation of PrivateAnalyzer
  AST::Context::ASTContext &getASTContext() { return astContext; }
  AnalyzerScope &getCurrentScope() { return scopeHandler.getCurrent(); }
  AnalyzerGlobalScope &getGlobalScope() { return scopeHandler.getGlobal(); }
};

} // namespace Analyzer