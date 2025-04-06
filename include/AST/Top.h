
#pragma once

#include <AST/AST.h>
#include <AST/Declaration.h>
#include <llvm-19/llvm/IR/Value.h>

namespace AST {

struct Top : public AST {
  llvm::Value *codegen() {
    std::vector<llvm::Value*> values;
    for (auto declaration : declarations) {
      values.emplace_back(declaration.codegen());
    }
    // For now do nothing
    return nullptr;
  }

  std::vector<Declaration::Declaration> declarations;
  std::vector<Declaration::Declaration> directives;
  //   std::vector<Declaration::Declaration> functions;
};

} // namespace AST::Top