#include "Support/Result.h"
#include <IR/IRGenerator.h>

// llvm
#include <llvm/IR/Instructions.h>

namespace IR {

Result<llvm::Value *> IRGenerator::handle(Assignment::Assignment &AST) {

  if (builder == nullptr) {
    return nullptr;
  }

  llvm::Value *assignmentTarget = nullptr;
  if (std::holds_alternative<VariableDeclaration::VariableDeclaration *>(
          AST.target)) {

    // Is this actually reachable? Or will all variable declarations already be
    // done beforehand?
    auto *variableDeclaration =
        std::get<VariableDeclaration::VariableDeclaration *>(AST.target);
    auto result = handle(*variableDeclaration);

    RET_ON_FAILURE(result,
                   "IRGenerator: Assignment: variable declaration error");

    assignmentTarget = *result;

  } else if (std::holds_alternative<Types::NamedIdentifier *>(AST.target)) {

    auto *name = std::get<Types::NamedIdentifier *>(AST.target);

    // TODO: move the getDeclaredVariableParentScope functionality to common
    // Scope. Because now this can only get variables that are in the current
    // scope.

    // Make sure the pointer of the variable is used here, it is needed for the
    // store.
    auto variable =
        scopeHandler.getCurrent().getVisibleDeclaredVariable(name->value);

    // Variable not declared.
    RET_ON_FALSE(
        variable.has_value(),
        "IRGenerator: Assignment: name identifier, variable note delcared");

    assignmentTarget = variable->getHighestOrderValue(builder);
  }

  auto exp = handle(*AST.expression);

  RET_ON_FAILURE(exp, "IRGenerator: Assignment: expression failure");

  return builder->CreateStore(*exp, assignmentTarget);
}

} // namespace IR