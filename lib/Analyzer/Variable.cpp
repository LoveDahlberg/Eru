#include <AST/Statement.h>
#include <Analyzer/Analyzer.h>
#include <Support/Result.h>

namespace Analyzer {

Result<bool> VariableAnalyzer::isTypeCheckedVariableDeclared(
    AST::VariableDeclaration::Variable *variable, bool checkParents) {

  auto &scope = analyzer.getCurrentScope();

  auto declaredVariable =
      checkParents ? scope.getDeclaredVariableParentScope(
                         variable->name, scope.getParentScope())
                   : scope.getDeclaredVariableCurrentScope(variable->name);

  if (!declaredVariable.has_value()) {
    return false;
  }

  RET_ON_NOT_EQUAL((*declaredVariable)->type, variable->type,
                   "isTypeCheckedVariableDeclared: Found matching variable "
                   "but type does not match.");
  return true;
}

bool VariableAnalyzer::isVariableDeclaredGlobally(
    AST::VariableDeclaration::Variable *variable) {
  auto result = isTypeCheckedVariableDeclared(variable, true);

  // If check failed or is true, then it means it exist in some form in scope.
  // Return true, otherwise false.
  return result.hasFailed || *result;
}

Error VariableAnalyzer::addVariableDeclarationInCurrentScope(
    AST::VariableDeclaration::Variable *variable) {

  auto result = isTypeCheckedVariableDeclared(variable, false);
  RET_ON_FAILURE(result, "addVariableDeclarationToCurrentScope: Failed "
                         "checking if variable is declared.");

  RET_ON_TRUE(
      *result,
      "addVariableDeclarationToCurrentScope: Variable already declared");

  analyzer.getCurrentScope().addVariableDeclaration(variable->name, variable);
  return SUCCESS;
}

Error VariableAnalyzer::ActOnGlobalDeclaration(
    AST::VariableDeclaration::Variable *variable,
    std::optional<AST::Expression::ConstantOperand> constOperand) {

  RET_ON_FALSE(analyzer.currentScopeIsGlobal(),
               "ActOnGlobalDeclaration: current scope is not global.");

  auto declaration = declareVariable(variable);
  RET_ON_FAILURE(
      declaration,
      "ActOnGlobalDeclaration: failed to verify variable declaration.");

  auto *globalVariableInitialization =
      new AST::VariableDeclaration::GlobalVariableInitialization(*declaration);

  if (constOperand.has_value()) {
    globalVariableInitialization->constOperand = *constOperand;
  }

  analyzer.getASTContext().compilationUnit->AddCompilationUnitItems(
      globalVariableInitialization);
  return SUCCESS;
}

Result<AST::VariableDeclaration::VariableDeclaration *>
VariableAnalyzer::ActOnLocalDeclaration(
    AST::VariableDeclaration::Variable *variable) {

  RET_ON_TRUE(analyzer.currentScopeIsGlobal(),
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
  RET_ON_FAILURE(addVariableDeclarationInCurrentScope(variable),
                 "verifyVariableDeclaration: failed addVariableDeclaration");

  auto declaration = new AST::VariableDeclaration::VariableDeclaration(
      variable, analyzer.currentScopeIsGlobal());

  // Check if variable is declared in any of the parent scopes. If it is, set
  // variable as hidingParentDeclaration. For now allow redefining of type.
  if (isVariableDeclaredGlobally(variable)) {
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
    RET_ON_FALSE(isVariableDeclaredGlobally(variable),
                 "ActOnAssignment: variable not previously declared.");

  } else if (std::holds_alternative<AST::Types::NamedIdentifier *>(target)) {

    auto result = analyzer.getCurrentScope().getVisibleDeclaredVariable(
        std::get<AST::Types::NamedIdentifier *>(target)->value);

    // Verify that the identifier matches with something that was previously
    // declared.
    RET_ON_FALSE(result.has_value(),
                 "ActOnAssignment: identifier not previously declared.");

    variable = *result;
  }

  // TODO add failure case.
  auto &evaluatedType = assignment->expression->evaluatedType;
  if (variable->type.isPointer) {
    RET_ON_FALSE(evaluatedType.dataType == INT,
                 "Cannot assign '" + variable->name + "' of type" +
                     variable->type.toPrintableString() +
                     "to an expression of type " + evaluatedType.toPrintableString() +
                     ".");
    evaluatedType.isPointer = true;
    return SUCCESS;
  }

  // Check that the type of the expression matches the type of the variable.
  RET_ON_NOT_EQUAL(
      evaluatedType, variable->type,
      "ActOnAssignment: type mismatch between declared variable " +
          variable->type.toPrintableString()+ " and assignment expression " +
          evaluatedType.toPrintableString() + ".");

  return SUCCESS;
}

} // namespace Analyzer