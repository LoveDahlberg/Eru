
#include "AST/Declaration.h"
#include <AST/Top.h>

using namespace AST;

std::vector<llvm::Value *> Top::codegen(llvm::Module& module) {
  std::vector<llvm::Value *> values;
  for (auto declaration : declarations) {
    values.emplace_back(declaration->codegen(module));
  }

  for (auto function : functions) {
    values.emplace_back(function->codegen(module));
  }
  return values;
}