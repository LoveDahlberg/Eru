#include <AST/Statement.h>
#include <Analyzer/Analyzer.h>

namespace Analyzer {

Result<bool> VariableAnalyzer::isTypeCheckedVariableDeclared(
    AST::VariableDeclaration::Variable *variable, AnalyzerScope &scope) {

  auto declaredVariable = scope.getDeclaredVariable(variable->name);

  if (declaredVariable == nullptr) {
    return false;
  }

  RET_ON_NOT_EQUAL(declaredVariable->type, variable->type,
                   "isTypeCheckedVariableDeclared: Found matching variable "
                   "but type does not match.");
  return true;
}

bool VariableAnalyzer::isVariableDeclared(
    AST::VariableDeclaration::Variable *variable, AnalyzerScope &scope) {
  auto result = isTypeCheckedVariableDeclared(variable, scope);

  // If check failed or is true, then it means it exist in some form in scope.
  // Return true, otherwise false.
  return result.hasFailed || *result;
}

AST::VariableDeclaration::Variable *
VariableAnalyzer::getDeclaredVariable(AST::Types::NamedIdentifier &identifier) {

  // Check current scope.
  auto currentScopeVariable =
      analyzer.getCurrentScope().getDeclaredVariable(identifier.value);

  // If found in current scope, return. Otherwise recursively check all parents
  // for it.
  return currentScopeVariable != nullptr
             ? currentScopeVariable
             : getDeclaredVariableParentScope(identifier,
                                              analyzer.getCurrentScope());
}

Error VariableAnalyzer::addVariableDeclarationToCurrentScope(
    AST::VariableDeclaration::Variable *variable) {

  auto result =
      isTypeCheckedVariableDeclared(variable, analyzer.getCurrentScope());
  RET_ON_FAILURE(result, "addVariableDeclarationToCurrentScope: Failed "
                         "checking if variable is declared.");

  RET_ON_TRUE(
      *result,
      "addVariableDeclarationToCurrentScope: Variable already declared");

  analyzer.getCurrentScope().variableDeclarations.emplace(variable->name,
                                                          variable);
  return SUCCESS;
}

bool VariableAnalyzer::isVariableDeclaredParentScope(
    AST::VariableDeclaration::Variable *variable, AnalyzerScope &scope) {

  // If no parent, then it is never declared in parent scope.
  if (scope.parentScope == nullptr) {
    return false;
  }

  // Check declaration in parent. If its found return true.
  // Else recursively check grandparents
  return isVariableDeclared(variable, *scope.parentScope)
             ? true
             : isVariableDeclaredParentScope(variable, *scope.parentScope);
}

AST::VariableDeclaration::Variable *
VariableAnalyzer::getDeclaredVariableParentScope(
    AST::Types::NamedIdentifier &identifier, AnalyzerScope &scope) {

  // If no parent, then it is never declared in parent scope.
  if (scope.parentScope == nullptr) {
    return nullptr;
  }

  // Check declaration in parent. If its found return it.
  // Else recursively check grandparents
  auto *variable = scope.getDeclaredVariable(identifier.value);
  return variable != nullptr
             ? variable
             : getDeclaredVariableParentScope(identifier, *scope.parentScope);
}

Error VariableAnalyzer::ActOnGlobalDeclaration(
    AST::VariableDeclaration::Variable *variable) {

  RET_ON_FALSE(analyzer.getCurrentScope().isGlobal(),
               "ActOnGlobalDeclaration: current scope is not global.");

  auto declaration = declareVariable(variable);
  RET_ON_FAILURE(
      declaration,
      "ActOnGlobalDeclaration: failed to verify variable declaration.");

  analyzer.getASTContext().compilationUnit->AddCompilationUnitItems(
      *declaration);
  return SUCCESS;
}

Result<AST::VariableDeclaration::VariableDeclaration *>
VariableAnalyzer::ActOnLocalDeclaration(
    AST::VariableDeclaration::Variable *variable) {

  RET_ON_TRUE(analyzer.getCurrentScope().isGlobal(),
              "ActOnLocalDeclaration: current scope is not local.");

  auto declaration = declareVariable(variable);
  RET_ON_FAILURE(
      declaration,
      "ActOnLocalDeclaration: failed to verify variable declaration.");

  return declaration;
}

Result<AST::VariableDeclaration::VariableDeclaration *>
VariableAnalyzer::declareVariable(
    AST::VariableDeclaration::Variable *variable) {

  // Check if variable is already declared in current scope.
  // If it is not, add the variable to the current scope.
  RET_ON_FAILURE(addVariableDeclarationToCurrentScope(variable),
                 "verifyVariableDeclaration: failed addVariableDeclaration");

  auto declaration = new AST::VariableDeclaration::VariableDeclaration(
      variable, analyzer.getCurrentScope().isGlobal());

  // Check if variable is declared in any of the parent scopes. If it is, set
  // variable as hidingParentDeclaration. For now allow redefining of type.
  if (isVariableDeclaredParentScope(variable, analyzer.getCurrentScope())) {
    declaration->hidingParentDeclaration = true;
  }

  return declaration;
}

Error VariableAnalyzer::ActOnAssignment(
    AST::Assignment::Assignment *assignment) {

  auto target = assignment->target;

  AST::VariableDeclaration::Variable *variable;

  // If variable declaration is the target of the assignment.
  if (std::holds_alternative<AST::VariableDeclaration::VariableDeclaration *>(
          target)) {

    variable = std::get<AST::VariableDeclaration::VariableDeclaration *>(target)
                   ->variable;

    // Verify that the target variable actually was declared previously.
    RET_ON_FALSE(isVariableDeclared(variable, analyzer.getCurrentScope()),
                 "ActOnAssignment: variable not previously declared.");

  } else if (std::holds_alternative<AST::Types::NamedIdentifier *>(target)) {

    variable = analyzer.getCurrentScope().getDeclaredVariable(
        std::get<AST::Types::NamedIdentifier *>(target)->value);

    // Verify that the identifier matches with something that was previously
    // declared.
    RET_ON_EQUAL(variable, nullptr,
                 "ActOnAssignment: identifier not previously declared.");
  }

  // Check that the type of the expression matches the type of the variable.
  RET_ON_NOT_EQUAL(assignment->expression->evaluatedType, variable->type,
                   "ActOnAssignment: type mismatch between declared variable "
                   "and assignment expression.");

  return SUCCESS;
}

} // namespace Analyzer