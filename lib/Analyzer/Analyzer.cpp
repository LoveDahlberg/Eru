#include "AST/Function.h"
#include <Analyzer/Analyzer.h>
#include <algorithm>

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

  for (auto *existingFunction : functions) {

    // For now, no function overloading to keep symbols easy to handle and look
    // at.

    // Check if the name matches what we have stored.
    if (existingFunction->name == function->name) {

      // TODO add asserts instead of runtime checks.
      RET_ON_EQUAL(
          existingFunction->definitionStatus,
          AST::Function::FunctionStatus::NONE,
          "addFunction: matching function has NONE set in FunctionStatus.");

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
                   "addFunction: Function with the same name but with "
                   "different parameters already declared or defined.");

      // Check what kind of FunctionStatus variant we are trying to add.
      switch (variant) {

      // Adding a function from its call.
      case AST::Function::CALL: {
        // Do nothing and retrn true. Here we are trying to add a call to
        // something that is either already marked as called or its declared or
        // defined (in which case we don't want to 'downgrade' it so to say)
        // already.

        // Note that we should not break here, as we cannot check the return
        // type from a call only.
        return true;
      }

      // Adding a function from its declaration.
      case AST::Function::DECLARATION: {

        // If the function is stored as a call.
        if (existingFunction->definitionStatus ==
            AST::Function::FunctionStatus::CALL) {

          // This means that the function to declare was previously called
          // without any known targets. We've now found its declaration, so it
          // can be promoted and we can store the return type (this is not known
          // from the call only).
          existingFunction->definitionStatus =
              AST::Function::FunctionStatus::DECLARATION;
          existingFunction->type = function->type;
          return true;
        }

        // Do nothing if the function is stored as a previous declaration or
        // definition.
        break;
      }

      // Adding a function from its definition.
      case AST::Function::DEFINITION: {

        // If the function is stored as a definition, error.
        RET_ON_EQUAL(existingFunction->definitionStatus,
                     AST::Function::FunctionStatus::DEFINITION,
                     "addFunction: Function defined multiple times in the same "
                     "compilation unit.");

        // Promote the status to to definition for when it previously was stored
        // as a call or a declaration.
        existingFunction->definitionStatus =
            AST::Function::FunctionStatus::DEFINITION;

        // Else if the function is stored as a call.
        if (existingFunction->definitionStatus ==
            AST::Function::FunctionStatus::CALL) {

          // This means that the function to define was previously called
          // without any known targets. We've now found its definition, so we
          // can store the return type (this is not known from the call only).
          existingFunction->type = function->type;
          return true;
        }

        break;
      }
      default:
        return {"addFunction: called with NONE FunctionStatus."};
      }

      // Check if the return type matches when we expect there to be a type set
      // in the stored function. if it doesn't its an error.
      RET_ON_NOT_EQUAL(existingFunction->type, function->type,
                       "addFunction: Function with the same name but with "
                       "a different type already declared.");

      return true;
    }
  }

  // Function is not declared, defined or called previously. Add it.
  function->definitionStatus = AST::Function::FunctionStatus::DECLARATION;
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

} // namespace Analyzer