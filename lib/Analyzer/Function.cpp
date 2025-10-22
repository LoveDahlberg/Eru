#include <Analyzer/Analyzer.h>

// stl
#include <algorithm>

namespace Analyzer {

Error FunctionAnalyzer::addFunction(AST::Function::Function *function,
                                    AST::Function::FunctionStatus variant) {

  // TODO make this a concept/template check, so that its not callable with
  // NONE.
  RET_ON_EQUAL(variant, AST::Function::FunctionStatus::NONE,
               "addFunction: variant has with definitionStatus NONE");

  auto& globalScope = analyzer.getGlobalScope();

  auto existingFunction = globalScope.getFunctionDeclaration(function->name);

  // Function does not exist.
  if (!globalScope.functionDeclarations.contains(function->name)) {
    function->definitionStatus = variant;
    globalScope.functionDeclarations.emplace(function->name, function);

    return SUCCESS;
  }

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

  // Could also just emit a warning and continue, the definition will take
  // prio.
  RET_ON_TRUE(existingFunction->definitionStatus ==
                      AST::Function::FunctionStatus::DECLARATION &&
                  variant == AST::Function::FunctionStatus::DEFINITION,
              "addFunction: function was previously declared as an "
              "external, but is now also defined.");

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

Error FunctionAnalyzer::ActOnDeclaration(AST::Function::Function *function) {

  // Must be in global scope to declare a function
  RET_ON_FALSE(analyzer.getCurrentScope().isGlobal(),
               "ActOnFunctionDeclaration: Function cannot be declared in other "
               "functions.");

  RET_ON_FAILURE(addFunction(function, AST::Function::DECLARATION),
                 "ActOnFunctionDeclaration: Failed to add function");

  analyzer.getASTContext().compilationUnit->AddCompilationUnitItems(function);
  return SUCCESS;
}

Error FunctionAnalyzer::ActOnDefinition(AST::Function::Function *function) {

  // Must be in global scope to define a new function
  RET_ON_FALSE(analyzer.getCurrentScope().isGlobal(),
               "ActOnFunctionDefinition: Function cannot be defined in other "
               "functions.");

  RET_ON_FAILURE(addFunction(function, AST::Function::DEFINITION),
                 "ActOnFunctionDeclaration: Failed to add function");

  analyzer.getASTContext().compilationUnit->AddCompilationUnitItems(function);
  return SUCCESS;
}

Result<AST::Function::Function *>
FunctionAnalyzer::ActOnCall(AST::Function::FunctionCall *call) {
  auto existingFunction =
      analyzer.getGlobalScope().getFunctionDeclaration(call->name);

  RET_ON_EQUAL(existingFunction, nullptr,
               "ActOnCall: function does not exist.");

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

Result<>
FunctionAnalyzer::ActOnParameters(AST::Function::Parameters parameters) {
  for (auto parameter : parameters) {
    RET_ON_FAILURE(analyzer.variable().ActOnLocalDeclaration(parameter),
                   "ActOnParameters: Failed to declare parameter as local "
                   "variable in scope.");
  }
  return SUCCESS;
}

} // namespace Analyzer