#pragma once

#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

namespace AST {
struct AST {
  virtual llvm::Value *codegen(llvm::Module& module) = 0;
};

struct GeneratingAST {
  virtual std::vector<llvm::Value *> codegen(llvm::Module &module) = 0;
};

} // namespace AST