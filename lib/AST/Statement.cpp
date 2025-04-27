
#include <AST/Statement.h>

namespace AST::Statement {

std::vector<llvm::Value *> Statement::codegen(llvm::Module &module) {
  return {};
}

} // namespace AST::Statement