
#pragma once

#include <AST/AST.h>
#include <AST/Declaration.h>
#include <llvm-19/llvm/IR/Value.h>

namespace AST {

struct Top {
  std::vector<llvm::Value*> codegen();

  std::vector<Declaration::Declaration> declarations;
  std::vector<Declaration::Declaration> directives;
  //   std::vector<Declaration::Declaration> functions;
};

} // namespace AST::Top