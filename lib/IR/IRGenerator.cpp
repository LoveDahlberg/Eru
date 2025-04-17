
// include
#include "llvm/IR/Metadata.h"
#include <IR/IRGenerator.h>

// stl
#include <iostream>

// llvm
#include <llvm/IR/Module.h>

namespace IR {

void GenerateIR(AST::Top top, llvm::Module& module) {
  auto values = top.codegen(module);
  std::cout << "GenerateIR\n";
  for (auto value : values) {
    value->print(llvm::outs(), false);
    std::cout << "\n";
  }
}

} // namespace IR