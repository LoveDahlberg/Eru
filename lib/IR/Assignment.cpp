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

    auto *variableDeclaration =
        std::get<VariableDeclaration::VariableDeclaration *>(AST.target);
    assignmentTarget = handle(*variableDeclaration);

  } else if (std::holds_alternative<Types::NamedIdentifier*>(AST.target)) {
    // Find alloc and set it as the assignment target.
  }

  auto exp = handle(*AST.expression);

  return builder->CreateStore(exp, assignmentTarget);
}

} // namespace IR