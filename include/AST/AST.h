#pragma once

#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/NoFolder.h>

#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

namespace AST {

struct codeGenItems
{
  codeGenItems(llvm::Module& module) : module(module) {}
  llvm::Module& module;
  llvm::IRBuilder<llvm::NoFolder> *builder = nullptr;
  llvm::Function *currentFunction = nullptr;
};

struct AST {
  virtual llvm::Value *codegen(codeGenItems& items) = 0;
};

struct GeneratingAST {
  virtual std::vector<llvm::Value *> codegen(codeGenItems& items) = 0;
};

} // namespace AST