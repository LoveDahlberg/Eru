// include
#include <AST/Declaration.h>
#include <AST/Types.h>
#include <IR/IRGenerator.h>

// llvm
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

// getst
#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

using namespace IR;
using namespace AST;

TEST(IR, testDeclaration) {

  auto ctx = new llvm::LLVMContext();
  auto module = llvm::Module("", *ctx);
  CompilationUnit compilationUnit;

  constexpr const char *variableName = "firstVariable";
  compilationUnit.AddTopConstruct(new Declaration::VariableDeclaration(
      (llvm::Type *)llvm::Type::getInt32Ty(module.getContext()), variableName));

  constexpr const char *functionName = "firstFunctionDeclaration";
  compilationUnit.AddTopConstruct(new Declaration::FunctionDeclaration(
      (llvm::Type *)llvm::Type::getInt1Ty(module.getContext()), functionName,
      {new Declaration::VariableDeclaration(
           (llvm::Type *)llvm::Type::getInt32Ty(module.getContext()),
           "firstParameter"),
       new Declaration::VariableDeclaration(
           (llvm::Type *)llvm::Type::getInt1Ty(module.getContext()),
           "firstParameter")}));

  GenerateIR(compilationUnit, module);

  auto generatedVariable = module.getGlobalVariable(variableName, true);
  EXPECT_NE(generatedVariable, nullptr);

  auto generatedFunction = module.getFunction(functionName);
  EXPECT_NE(generatedFunction, nullptr);
}