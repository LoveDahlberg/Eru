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

    // Make sure the pointer of the variable is used here, it is needed for the
    // store.
    auto variable =
        scopeHandler.getCurrent().getVisibleDeclaredVariable(name->value);

    // Variable not declared.
    RET_ON_FALSE(
        variable.has_value(),
        "IRGenerator: Assignment: name identifier, variable note delcared");

    assignmentTarget = (*variable)->getAddress(builder);
  }

  auto maybeExp = handle(*AST.expression);

  RET_ON_FAILURE(maybeExp, "IRGenerator: Assignment: expression failure");

  // Need to create inttoptr to tell llvm that this expression is casted to a
  // pointer.
  auto exp = [&]() {
    if (AST.expression->evaluatedType.isPointer) {
      return builder->CreateIntToPtr(*maybeExp, builder->getPtrTy());
    }
    return *maybeExp;
  }();

  return builder->CreateStore(exp, assignmentTarget);
}

} // namespace IR