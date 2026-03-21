#include <AST/Statement.h>
#include <Analyzer/Analyzer.h>
#include <Support/Result.h>
#include <string>

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
  return ERU_SUCCESS;
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
  return ERU_SUCCESS;
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

Error VariableAnalyzer::ActOnAssignment(AST::Assignment::Assignment *assignment,
                                        int indirection) {

  auto target = assignment->target;
  auto &expressionType = assignment->expression->evaluatedType;

  AST::VariableDeclaration::Variable *assignmentTarget;

  // If variable declaration is the target of the assignment.
  if (std::holds_alternative<AST::VariableDeclaration::VariableDeclaration *>(
          target)) {

    assignmentTarget =
        std::get<AST::VariableDeclaration::VariableDeclaration *>(target)
            ->variable;

    // Verify that the target variable actually was declared previously.
    RET_ON_FALSE(isVariableDeclaredGlobally(assignmentTarget),
                 "ActOnAssignment: variable not previously declared.");

  } else if (std::holds_alternative<AST::Types::NamedIdentifier *>(target)) {

    auto result = analyzer.getCurrentScope().getVisibleDeclaredVariable(
        std::get<AST::Types::NamedIdentifier *>(target)->value);

    // Verify that the identifier matches with something that was previously
    // declared.
    RET_ON_FALSE(result.has_value(),
                 "ActOnAssignment: identifier not previously declared.");

    assignmentTarget = *result;
  }

  const auto assignmentError =
      "Cannot assign '" + assignmentTarget->name + "' of type" +
      assignmentTarget->type.toPrintableString(true, indirection) +
      "to an expression of type " + expressionType.toPrintableString() + ".";

  // Evaluate pointer indirection correctness.
  if (assignmentTarget->type.isPointer) {

    // There is indirection on the taret being assigned.
    if (indirection != 0) {
      // Save indirection steps for IR generation.
      assignment->indirectionSteps = indirection;

      const auto &remaningSteps =
          assignmentTarget->type.pointerDepth - indirection;

      RET_ON_TRUE(remaningSteps < 0,
                  "Cannot dereference '" + assignmentTarget->name + "' " +
                      std::to_string(indirection) + " times, only " +
                      std::to_string(assignmentTarget->type.pointerDepth) +
                      " times is possible for type" +
                      assignmentTarget->type.toPrintableString());

      // Inidrection is all the way down, we need to typecheck the
      // varaible->type.dataType and evaluatedType.dataType
      if (remaningSteps == 0) {
        RET_ON_NOT_EQUAL(
            assignmentTarget->type.dataType, expressionType.dataType,
            "Cannot assign dereferenced '" + assignmentTarget->name +
                "' of type" +
                assignmentTarget->type.toPrintableString(true, indirection) +
                "to an expression of type " +
                expressionType.toPrintableString() + ".");
        return ERU_SUCCESS;
      }

      // Else indirection is not all the way down to the type, so we need to
      // typecheck the expression pointer.
    }

    // Assignment target is a pointer, veryify that expression is an int. We
    // currently do not see any difference between ints and pointer type wise.
    RET_ON_FALSE(expressionType.dataType == INT, assignmentError);
    expressionType.isPointer = true;
    return ERU_SUCCESS;
  }

  // Check that the type of the expression matches the type of the variable.
  RET_ON_NOT_EQUAL(expressionType, assignmentTarget->type, assignmentError);

  return ERU_SUCCESS;
}

} // namespace Analyzer