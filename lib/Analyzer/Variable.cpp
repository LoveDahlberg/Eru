#include <AST/Statement.h>
#include <Analyzer/Analyzer.h>

namespace Analyzer {

Result<bool> Analyzer::isTypeCheckedVariableDeclared(
    AST::VariableDeclaration::Variable *variable,
    std::unique_ptr<Scope> &scope) {
  for (auto *declaredVarible : scope->variableDeclarations) {

    // Found matching declaration in scope.
    if (declaredVarible->name == variable->name) {

      RET_ON_NOT_EQUAL(declaredVarible->type, variable->type,
                       "isVariableDeclared: Found matching variable but type "
                       "does not match.");
      return true;
    }
  }
  return false;
}

// TODO: rewrite in std::ranges get_if or something
AST::VariableDeclaration::Variable *
Analyzer::getDeclaredVariable(AST::Types::NamedIdentifier *identifier,
                              std::unique_ptr<Scope> &scope) {
  for (auto *declaredVarible : scope->variableDeclarations) {
    if (declaredVarible->name == identifier->value) {
      return declaredVarible;
    }
  }
  return nullptr;
}

bool Analyzer::isVariableDeclared(AST::VariableDeclaration::Variable *variable,
                                  std::unique_ptr<Scope> &scope) {
  auto result = isTypeCheckedVariableDeclared(variable, scope);

  // If check failed or is true, then it means it exist in some form in scope.
  // Return true, otherwise false.
  return result.hasFailed || *result;
}

Result<bool> Analyzer::addVariableDeclarationToCurrentScope(
    AST::VariableDeclaration::Variable *variable) {

  auto result = isTypeCheckedVariableDeclared(variable, currentScope);
  RET_ON_FAILURE(
      result,
      "addVariableDeclaration: Failed checking if variable is declared.");

  RET_ON_TRUE(*result, "addVariableDeclaration: Variable already declared");

  currentScope->variableDeclarations.push_back(variable);
  return true;
}

bool Analyzer::isVariableDeclaredParentScope(
    AST::VariableDeclaration::Variable *variable) {

  // If no parent, then it is never declared in parent scope.
  if (currentScope->parentScope == nullptr) {
    return false;
  }

  // Else check declaration in parent.
  return isVariableDeclared(variable, currentScope->parentScope);
}

Result<bool> Analyzer::ActOnGlobalVariableDeclaration(
    AST::VariableDeclaration::Variable *variable) {

  RET_ON_FALSE(currentScope->isGlobal,
               "ActOnGlobalVariableDeclaration: current scope is not global.");

  auto declaration = declareVariable(variable);
  RET_ON_FAILURE(
      declaration,
      "ActOnGlobalVariableDeclaration: failed to verify variable declaration.");

  astContext.compilationUnit->AddCompilationUnitItems(*declaration);
  return true;
}

Result<bool> Analyzer::ActOnLocalVariableDeclaration(
    AST::VariableDeclaration::Variable *variable,
    AST::Statement::Statement *statement) {

  RET_ON_TRUE(currentScope->isGlobal,
              "ActOnLocalVariableDeclaration: current scope is not local.");

  auto declaration = declareVariable(variable);
  RET_ON_FAILURE(
      declaration,
      "ActOnLocalVariableDeclaration: failed to verify variable declaration.");

  statement->AddStatement(*declaration);
  return true;
}

Result<AST::VariableDeclaration::VariableDeclaration *>
Analyzer::declareVariable(AST::VariableDeclaration::Variable *variable) {

  // Check if variable is already declared in current scope.
  // If it is not, add the variable to the current scope.
  RET_ON_FAILURE(addVariableDeclarationToCurrentScope(variable),
                 "verifyVariableDeclaration: failed addVariableDeclaration");

  auto declaration = new AST::VariableDeclaration::VariableDeclaration(
      variable, currentScope->isGlobal);

  // Check if variable is declared in any of the parent scopes. If it is, set
  // variable as hidingParentDeclaration. For now allow redefining of type.
  if (isVariableDeclaredParentScope(variable)) {
    declaration->hidingParentDeclaration = true;
  }

  return declaration;
}

Result<bool> Analyzer::ActOnAssignment(AST::Assignment::Assignment *assignment,
                                       AST::Statement::Statement *statement) {

  auto target = assignment->target;

  AST::VariableDeclaration::Variable *variable;

  // If variable declaration is the target of the assignment.
  if (std::holds_alternative<AST::VariableDeclaration::VariableDeclaration *>(
          target)) {

    variable = std::get<AST::VariableDeclaration::VariableDeclaration *>(target)
                   ->variable;

    // Verify that the target variable actually was declared previously.
    RET_ON_FALSE(isVariableDeclared(variable, currentScope),
                 "ActOnAssignment: variable not previously declared.");

  } else if (std::holds_alternative<AST::Types::NamedIdentifier *>(target)) {

    variable = getDeclaredVariable(
        std::get<AST::Types::NamedIdentifier *>(target), currentScope);

    // Verify that the identifier matches with something that was previously
    // declared.
    RET_ON_EQUAL(variable, nullptr,
                 "ActOnAssignment: identifier not previously declared.");
  }

  RET_ON_FAILURE(ActOnExpression(assignment->expression, variable->type),
                 "ActOnAssignment: failed to act on expression");

  statement->AddStatement(assignment);

  return true;
}

} // namespace Analyzer