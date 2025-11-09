#pragma once

#include <AST/Function.h>
#include <AST/Types.h>
#include <AST/VariableDeclaration.h>
#include <Support/Scope.h>

using namespace Support::Scope;

namespace Analyzer {

/// Extra information needed in the scope.
struct AnalyzerScopeContextData {
  AST::Function::FunctionDeclaration declaration;
};

using AnalyzerScopeHandler =
    ScopeHandler<AST::VariableDeclaration::Variable *,
                 AST::Function::FunctionDeclaration *, AnalyzerScopeContextData>;
using AnalyzerScope = Scope<AST::VariableDeclaration::Variable *>;
using AnalyzerLocalScope =
    LocalScope<AST::VariableDeclaration::Variable *, AnalyzerScopeContextData>;
using AnalyzerGlobalScope = GlobalScope<AST::VariableDeclaration::Variable *,
                                        AST::Function::FunctionDeclaration *>;
using AnalyzerFunctionScope =
    FunctionScope<AST::VariableDeclaration::Variable *,
                  AnalyzerScopeContextData>;
} // namespace Analyzer