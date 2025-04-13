// include
#include <AST/Types.h>
#include <IR/IRGenerator.h>

// llvm
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

// getst
#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>
#include <memory>

using namespace IR;
using namespace AST;

TEST(IR, TestGenerateIR) {

  auto ctx =  new llvm::LLVMContext();
  auto module = llvm::Module("", *ctx);

  Top top;
  top.declarations.push_back(new Declaration::VariableDeclaration(
      (llvm::Type *)llvm::Type::getInt32Ty(module.getContext()), "test"));

  GenerateIR(top, module);
}