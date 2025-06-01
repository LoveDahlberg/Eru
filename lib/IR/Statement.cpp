
#include <AST/Statement.h>

namespace AST::Statement {

llvm::Value *Statement::codegen(codeGenItems &items) {

  if (items.builder == nullptr) {
    return nullptr;
  }

  // Generate all ValidstatementType statements
  for (auto statement : statements) {
    auto segment = statement->codegen(items);
    if (segment == nullptr) {
      // err
      return nullptr;
    }
  }

  // TODO what to return here?
  return items.builder->GetInsertBlock();
}

} // namespace AST::Statement