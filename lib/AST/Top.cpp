
#include <AST/Top.h>

using namespace AST;

std::vector<llvm::Value *> Top::codegen(llvm::Module& module) {
  std::vector<llvm::Value *> values;
  for (auto constructs : topConstructs) {
    values.emplace_back(constructs->codegen(module));
  }
  return values;
}