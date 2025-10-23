#include <IR/IRGenerator.h>

// llvm
#include <llvm/IR/Instructions.h>

namespace IR {

llvm::Value *IRGenerator::handle(Assignment::Assignment &AST) {

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
    assignmentTarget = handle(*variableDeclaration);

  } else if (std::holds_alternative<Types::NamedIdentifier *>(AST.target)) {

    auto *name = std::get<Types::NamedIdentifier *>(AST.target);

    // TODO: move the getDeclaredVariableParentScope functionality to common
    // Scope. Because now this can only get variables that are in the current
    // scope.

    // Make sure the pointer of the variable is used here, it is needed for the
    // store.
    auto variable = scopeHandler.getCurrent().getDeclaredVariable(name->value);

    // Variable note declared.
    if (!variable.has_value()) {
      return nullptr;
    }

    assignmentTarget = variable->getHighestOrderValue(builder);
  }

  auto exp = handle(*AST.expression);

  return builder->CreateStore(exp, assignmentTarget);
}

} // namespace IR