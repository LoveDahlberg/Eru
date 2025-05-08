#include <AST/Controlflow.h>

namespace AST::Controlflow {

llvm::Value *ConditionalBranchingGroup::codegen(codeGenItems& items) {
  return nullptr;
}

llvm::Value *ConditionalBranch::codegen(codeGenItems& items) { return nullptr; }
} // namespace AST::Function