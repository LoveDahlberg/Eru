#include <AST/Assignment.h>
#include <llvm/IR/Instructions.h>

// stl
#include <variant>

namespace AST::Assignment {

llvm::Value *Assignment::codegen(codeGenItems &items) {

  if (items.builder == nullptr) {
    return nullptr;
  }

  llvm::Value *assignmentTarget = nullptr;
  if (std::holds_alternative<VariableDeclaration::VariableDeclaration *>(
          target)) {

    auto *variableDeclaration =
        std::get<VariableDeclaration::VariableDeclaration *>(target);
    assignmentTarget = variableDeclaration->codegen(items);

  } else if (std::holds_alternative<Types::NamedIdentifier>(target)) {
    // Find alloc and set it as the assignment target.
  }

  auto exp = expression->codegen(items);

  return items.builder->CreateStore(exp, assignmentTarget);
}

} // namespace AST::Assignment