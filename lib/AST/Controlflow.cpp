#include <AST/Controlflow.h>

namespace AST::Controlflow {

llvm::Value *ConditionalBranchingGroup::codegen(llvm::Module &module) {
  return nullptr;
}

llvm::Value *ConditionalBranch::codegen(llvm::Module &module) { return nullptr; }
} // namespace AST::Function