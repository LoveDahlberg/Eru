
// include
#include "AST/AST.h"
#include "llvm/IR/Metadata.h"
#include <IR/IRGenerator.h>

// stl
#include <iostream>

// llvm
#include <llvm/IR/Module.h>

namespace IR {

std::vector<llvm::Value *> GenerateIR(AST::CompilationUnit compilationUnit,
                                      llvm::Module &module) {

  AST::codeGenItems items(module);
  auto values = compilationUnit.codegen(items);
  std::cout << "GenerateIR\n";
  for (auto value : values) {
    value->print(llvm::outs(), false);
    std::cout << "\n";
  }
  return values;
}

} // namespace IR