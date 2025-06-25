#include <Analyzer/Analyzer.h>

namespace Analyzer {

bool Analyzer::declareVariableCurrentScope(
    AST::VariableDeclaration::Variable *variable) {
  return true;
}

bool Analyzer::isDeclaredVariableParentScope(
    AST::VariableDeclaration::Variable *variable) {
  return true;
}

void Analyzer::ActOnVariableDeclaration(
    AST::VariableDeclaration::Variable *variable) {

  // Check if variable is already declared in current scope.
  // If it is, redeclaration is not allowed.
  if (!declareVariableCurrentScope(variable)) {
    // err
    return;
  }

  auto declaration = new AST::VariableDeclaration::VariableDeclaration(
      variable, CurrentScope->isGlobal);

  // Check if variable is declared in any of the parent scopes. If it is, set
  // variable as hidingParentDeclaration. For now allow redefining of type.
  if (isDeclaredVariableParentScope(variable)) {
    declaration->hidingParentDeclaration = true;
  }

  astContext.compilationUnit->AddCompilationUnitItems(declaration);
}

bool Analyzer::declareFunction(AST::Function::Function *function) {
  return true;
}

void Analyzer::ActOnFunctionDeclaration(AST::Function::Function *function) {
  // Must be in global scope to declare a new function
  if (!CurrentScope->isGlobal) {
    // err
    return;
  }

  astContext.compilationUnit->AddCompilationUnitItems(function);
}

void Analyzer::ActOnFunctionImplementation(AST::Function::Function *function) {
  astContext.compilationUnit->AddCompilationUnitItems(function);
}

} // namespace Analyzer