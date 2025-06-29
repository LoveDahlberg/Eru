#include <Analyzer/Analyzer.h>

// stl
#include <algorithm>
#include <cassert>

namespace Analyzer {

// TODO add Result<bool> here.
bool Analyzer::isOkToDeclareVariable(
    AST::VariableDeclaration::Variable *variable,
    std::unique_ptr<Scope> &scope) {
  for (auto *declaredVarible : scope->variableDeclarations) {

    // Variable already exists in given scope.
    if (declaredVarible->name == variable->name) {
      if (declaredVarible->type != variable->type) {
        // Redeclaring variable but with different type.
        return false;
      }
      // Redeclaring varible same type
      return false;
    }
  }
  return true;
}

/// Check if variable is already declared in current scope.
bool Analyzer::addVariableDeclaration(
    AST::VariableDeclaration::Variable *variable) {

  if (!isOkToDeclareVariable(variable, currentScope)) {
    return false;
  }

  currentScope->variableDeclarations.push_back(variable);
  return true;
}

bool Analyzer::isDeclaredVariableParentScope(
    AST::VariableDeclaration::Variable *variable) {

  return currentScope->parentScope == nullptr
             ? false
             : isOkToDeclareVariable(variable, currentScope->parentScope);
}

Result<bool> Analyzer::ActOnVariableDeclaration(
    AST::VariableDeclaration::Variable *variable) {

  // Check if variable is already declared in current scope.
  // If it is, redeclaration is not allowed.

  if (!addVariableDeclaration(variable)) {
    return {"ActOnVariableDeclaration: Variable already declared in current "
            "scope."};
  }

  auto declaration = new AST::VariableDeclaration::VariableDeclaration(
      variable, currentScope->isGlobal);

  // Check if variable is declared in any of the parent scopes. If it is, set
  // variable as hidingParentDeclaration. For now allow redefining of type.
  if (isDeclaredVariableParentScope(variable)) {
    declaration->hidingParentDeclaration = true;
  }

  astContext.compilationUnit->AddCompilationUnitItems(declaration);
  return true;
}

Result<bool> Analyzer::addFunction(AST::Function::Function *function,
                                   AST::Function::FunctionStatus variant) {

  // TODO make this a concept/template check, so that its not callable with
  // NONE.
  RET_ON_EQUAL(variant, AST::Function::FunctionStatus::NONE,
               "addFunction: variant has with definitionStatus NONE");

  for (auto *existingFunction : functions) {

    // For now, no function overloading to keep symbols easy to handle and look
    // at.

    // Check if the name matches what we have stored.
    if (existingFunction->name == function->name) {

      RET_ON_EQUAL(
          existingFunction->definitionStatus,
          AST::Function::FunctionStatus::NONE,
          "addFunction: existingFunction has with definitionStatus NONE");

      RET_ON_TRUE(existingFunction->definitionStatus ==
                          AST::Function::FunctionStatus::DEFINITION &&
                          variant ==
                          AST::Function::FunctionStatus::DEFINITION,
                  "addFunction: Function defined multiple times in the same "
                  "compilation unit.");

      // Could also just emit a warning and continue, the definition will take
      // prio.
      RET_ON_TRUE(existingFunction->definitionStatus ==
                          AST::Function::FunctionStatus::DECLARATION &&
                          variant ==
                          AST::Function::FunctionStatus::DEFINITION,
                  "addFunction: function was previously declared as an "
                  "external, but is now also defined.");

      // Could also just emit a warning and continue, the definition will take
      // prio.
      RET_ON_TRUE(existingFunction->definitionStatus ==
                          AST::Function::FunctionStatus::DEFINITION &&
                          variant ==
                          AST::Function::FunctionStatus::DECLARATION,
                  "addFunction: function was previously defined, but is now "
                  "declared as an external.");

      // Both are declarations. This is ok as long as they match.

      // Compare the arguments of the two, if they don't match exactly its an
      // error (no overloading), return false.
      auto result =
          existingFunction->parameters.size() == function->parameters.size()
              ? std::ranges::equal(
                    existingFunction->parameters, function->parameters,
                    [](const AST::VariableDeclaration::Variable *lhs,
                       const AST::VariableDeclaration::Variable *rhs) {
                      return lhs->type == rhs->type;
                    })
              : false;

      RET_ON_EQUAL(result, false,
                   "addFunction: external function declaration with the same "
                   "name but with different parameters already declared.");

      // Check if the return type matches when we expect there to be a type
      // set in the stored function. if it doesn't its an error.
      RET_ON_NOT_EQUAL(existingFunction->type, function->type,
                       "addFunction: Function with the same name but with "
                       "a different type already declared.");

      return true;
    }
  }

  function->definitionStatus = variant;
  functions.push_back(function);

  return true;
}

Result<bool>
Analyzer::ActOnFunctionDeclaration(AST::Function::Function *function) {

  // Must be in global scope to declare a function
  RET_ON_FALSE(currentScope->isGlobal,
               "ActOnFunctionDeclaration: Function cannot be declared in other "
               "functions.");

  RET_ON_FAILURE(addFunction(function, AST::Function::DECLARATION),
                 "ActOnFunctionDeclaration: Failed to add function");

  astContext.compilationUnit->AddCompilationUnitItems(function);
  return true;
}

Result<bool>
Analyzer::ActOnFunctionDefinition(AST::Function::Function *function) {

  // Must be in global scope to define a new function
  RET_ON_FALSE(currentScope->isGlobal,
               "ActOnFunctionDefinition: Function cannot be defined in other "
               "functions.");

  RET_ON_FAILURE(addFunction(function, AST::Function::DEFINITION),
                 "ActOnFunctionDeclaration: Failed to add function");

  astContext.compilationUnit->AddCompilationUnitItems(function);
  return true;
}

Result<bool> Analyzer::ActOnFunctionCall(AST::Function::FunctionCall *call) {

  // RET_ON_FAILURE(addFunction(call, AST::Function::CALL),
  // "ActOnFunctionDeclaration: Failed to add function");
  return true;
}

} // namespace Analyzer