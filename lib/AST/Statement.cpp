
#include <AST/Statement.h>

namespace AST::Statement {

llvm::Value *Statement::codegen(codeGenItems &items) {

  if (items.builder == nullptr) {
    return nullptr;
  }

  for (auto statement : statements) {
    auto start = statement->codegen(items);
    if (start == nullptr) {
      return nullptr;
    }
  }

  // TODO what to return here?
  return items.builder->CreateRetVoid();
}

} // namespace AST::Statement