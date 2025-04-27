
#include <AST/CompilationUnit.h>

using namespace AST;

std::vector<llvm::Value *> CompilationUnit::codegen(llvm::Module& module) {
  std::vector<llvm::Value *> values;
  for (auto constructs : compilationUnitItems) {
    values.emplace_back(constructs->codegen(module));
  }
  return values;
}