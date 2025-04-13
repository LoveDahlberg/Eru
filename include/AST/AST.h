
#pragma once

#include <llvm/IR/Value.h>

namespace AST {

/// Tpo AST construct, logic that
struct AST {
  // virtual ~AST() = default;
  // virtual llvm::Value *codegen() = 0;
};

// class Directive : public AST {
//   llvm::Value *codegen() { return nullptr; }
// };

// class Function : public AST {

//   llvm::Value *codegen() { return nullptr; }
// };

} // namespace AST