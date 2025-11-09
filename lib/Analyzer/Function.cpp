#include <AST/Types.h>
#include <Analyzer/Analyzer.h>
#include <Analyzer/AnalyzerTypes.h>
#include <Support/Result.h>
#include <Support/Scope.h>

// stl
#include <algorithm>

namespace Analyzer {

Error FunctionAnalyzer::addFunction(
    AST::Function::FunctionDeclaration *function,
    AST::Function::FunctionStatus variant) {

  // TODO make this a concept/template check, so that its not callable with
  // NONE.
  RET_ON_EQUAL(variant, AST::Function::FunctionStatus::NONE,
               "addFunction: variant has with definitionStatus NONE");

  auto &globalScope = analyzer.getGlobalScope();

  auto maybeExistingFunction =
      globalScope.getFunctionDeclaration(function->name);

  // TODO change to instead be an error on definition. We always scan the file
  // for declarations first, so we should never find a definition that was not
  // previously declared.

  // Checks if function does not exist, and if not, add it.
  if (!maybeExistingFunction.has_value()) {
    function->definitionStatus = variant;
    globalScope.addFunctionDeclaration(function->name, function);

    return SUCCESS;
  }

  auto existingFunction = *maybeExistingFunction;

  // For now, no function overloading to keep symbols easy to handle and look
  // at.

  // Function is previously declared or defined.

  RET_ON_EQUAL(existingFunction->definitionStatus,
               AST::Function::FunctionStatus::NONE,
               "addFunction: existingFunction has with definitionStatus NONE");

  RET_ON_TRUE(existingFunction->definitionStatus ==
                      AST::Function::FunctionStatus::DEFINITION &&
                  variant == AST::Function::FunctionStatus::DEFINITION,
              "addFunction: Function defined multiple times in the same "
              "compilation unit.");

  // TODO consider if this should be removed when implementing the include
  // (link) system.

  // Could also just emit a warning and continue, the definition will take
  // prio.
  RET_ON_TRUE(existingFunction->definitionStatus ==
                      AST::Function::FunctionStatus::DECLARATION &&
                  variant == AST::Function::FunctionStatus::DEFINITION,
              "addFunction: function was previously declared as an "
              "external, but is now also defined.");

  // TODO consider if this should be removed when implementing the include
  // (link) system.

  // Could also just emit a warning and continue, the definition will take
  // prio.
  RET_ON_TRUE(existingFunction->definitionStatus ==
                      AST::Function::FunctionStatus::DEFINITION &&
                  variant == AST::Function::FunctionStatus::DECLARATION,
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

  return SUCCESS;
}

Error FunctionAnalyzer::ActOnDeclaration(
    AST::Function::FunctionDeclaration *function) {

  // Must be in global scope to declare a function
  RET_ON_FALSE(analyzer.currentScopeIsGlobal(),
               "ActOnFunctionDeclaration: Function cannot be declared in other "
               "functions.");

  RET_ON_FAILURE(addFunction(function, AST::Function::DECLARATION),
                 "ActOnFunctionDeclaration: Failed to add function");

  analyzer.getASTContext().compilationUnit->AddCompilationUnitItems(function);
  return SUCCESS;
}

Error FunctionAnalyzer::ActOnDefinition(
    AST::Function::FunctionDeclaration *function) {

  // Must be in global scope to define a new function
  RET_ON_FALSE(analyzer.currentScopeIsGlobal(),
               "ActOnFunctionDefinition: Function cannot be defined in other "
               "functions.");

  RET_ON_FAILURE(addFunction(function, AST::Function::DEFINITION),
                 "ActOnFunctionDeclaration: Failed to add function");

  analyzer.getASTContext().compilationUnit->AddCompilationUnitItems(function);
  return SUCCESS;
}

Result<AST::Function::FunctionDeclaration *>
FunctionAnalyzer::ActOnCall(AST::Function::FunctionCall *call) {
  auto maybeExistingFunction =
      analyzer.getGlobalScope().getFunctionDeclaration(call->name);

  RET_ON_FALSE(maybeExistingFunction.has_value(),
               "ActOnCall: function does not exist.");

  auto existingFunction = *maybeExistingFunction;

  auto numberOfParameters = existingFunction->parameters.size();
  RET_ON_NOT_EQUAL(numberOfParameters, call->parameters.size(),
                   "ActOnCall: number of parameters in declaration and call "
                   "does not match.");

  for (int i = 0; i < numberOfParameters; ++i) {

    RET_ON_NOT_EQUAL(
        call->parameters[i]->evaluatedType,
        existingFunction->parameters[i]->type,
        Formatter(
            "ActOnCall: parameter '", i, ":",
            AST::Types::typeToString.at(call->parameters[i]->evaluatedType),
            "' for '", call->name,
            "' does not match with the resulting type from expression '", i,
            ":",
            AST::Types::typeToString.at(existingFunction->parameters[i]->type),
            "' for '", existingFunction->name, "'."));
  }
  return existingFunction;
}

Error FunctionAnalyzer::ActOnCall(AST::Function::FunctionCall *call,
                                  AST::Types::Types expectedReturnValue) {

  RET_ON_TRUE(expectedReturnValue == AST::Types::NONE,
              "ActOnCall: expected return value cannot be NONE.");

  auto existingFunction = ActOnCall(call);

  RET_ON_FAILURE(
      existingFunction,
      "ActOnCall: failed to verify function declaration or parameters.");

  RET_ON_NOT_EQUAL(
      expectedReturnValue, (*existingFunction)->type,
      "ActOnCall: return value of function does not match the expected one.");

  return SUCCESS;
}

Error FunctionAnalyzer::ActOnParameters() {

  auto *currentLocalScope = analyzer.getLocalScope();
  RET_ON_EQUAL(currentLocalScope, nullptr,
               "ActOnParameters: current scope is not local.");

  auto *contextData = currentLocalScope->getContextData();
  RET_ON_EQUAL(contextData, nullptr, "ActOnParameters: context data not set.");

  auto &declaration = contextData->declaration;

  // Declare parameters when acting on a function.
  if (currentLocalScope->getScopeKind() ==
      Support::Scope::scopeKind::FUNCTION) {
    for (auto parameter : declaration.parameters) {
      RET_ON_FAILURE(analyzer.variable().ActOnLocalDeclaration(parameter),
                     "ActOnParameters: Failed to declare parameter as local "
                     "variable in scope.");
    }
  }

  return SUCCESS;
}

Error FunctionAnalyzer::ActOnReturnValue(AST::Types::Types returnValue) {
  auto *currentLocalScope = analyzer.getLocalScope();
  RET_ON_EQUAL(currentLocalScope, nullptr,
               "ActOnReturnValue: current scope is not local.");

  auto *contextData = currentLocalScope->getContextData();
  RET_ON_EQUAL(contextData, nullptr, "ActOnReturnValue: context data not set.");

  auto &declaration = contextData->declaration;
  auto errorMessage = "ActOnReturnValue: function " + declaration.name +
                      " declared as " +
                      AST::Types::typeToString.at(declaration.type) +
                      " , but found return of "
                      "type " +
                      AST::Types::typeToString.at(returnValue);

  // Check return type only when:
  // 1. At the end of a function.
  // 2. At the end of a local scope IF something is actually returned.
  if (currentLocalScope->getScopeKind() ==
          Support::Scope::scopeKind::FUNCTION ||
      (currentLocalScope->getScopeKind() == Support::Scope::scopeKind::LOCAL &&
       returnValue != AST::Types::Types::NONE)) {
    RET_ON_NOT_EQUAL(declaration.type, returnValue, errorMessage);
  }
  return SUCCESS;
}

Error FunctionAnalyzer::ActOnBody(AST::Function::FunctionBody *body) {
  auto maybeExistingFunction =
      analyzer.getGlobalScope().getFunctionDeclaration(body->functionName);

  RET_ON_FALSE(maybeExistingFunction.has_value(), "ActOnBody: function '" +
                                                      body->functionName +
                                                      "' does not exist.");

  analyzer.getASTContext().compilationUnit->AddCompilationUnitItems(body);

  return SUCCESS;
}

} // namespace Analyzer