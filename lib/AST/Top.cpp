
#include <AST/Top.h>

using namespace AST;

std::vector<llvm::Value *> Top::codegen(llvm::Module& module) {
  std::vector<llvm::Value *> values;
  for (auto declaration : declarations) {
    values.emplace_back(declaration->codegen(module));
  }
  return values;
}