// include
#include <AST/Function.h>
#include <AST/Statement.h>
#include <AST/Types.h>
#include <AST/VariableDeclaration.h>
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
  auto variable = new VariableDeclaration::VariableDeclaration(
      (llvm::Type *)llvm::Type::getInt32Ty(module.getContext()), variableName);
  variable->global = true;
  compilationUnit.AddCompilationUnitItems(variable);

  //   constexpr const char *functionName = "firstFunctionDeclaration";
  //   compilationUnit.AddCompilationUnitItems(new Function::Function(
  //       (llvm::Type *)llvm::Type::getInt1Ty(module.getContext()),
  //       functionName, {new VariableDeclaration::VariableDeclaration(
  //            (llvm::Type *)llvm::Type::getInt32Ty(module.getContext()),
  //            "firstParameter"),
  //        new VariableDeclaration::VariableDeclaration(
  //            (llvm::Type *)llvm::Type::getInt1Ty(module.getContext()),
  //            "firstParameter")}));

  GenerateIR(compilationUnit, module);

  auto generatedVariable = module.getGlobalVariable(variableName, true);
  EXPECT_NE(generatedVariable, nullptr);

  //   auto generatedFunction = module.getFunction(functionName);
  //   EXPECT_NE(generatedFunction, nullptr);
}

TEST(IR, testFunction) {
  auto ctx = new llvm::LLVMContext();
  auto module = llvm::Module("", *ctx);
  CompilationUnit compilationUnit;

  constexpr const char *name = "function";
  constexpr const char *variableName = "firstVariable";

  // Create statement and add a single variable declaration in it.
  auto statement = new Statement::Statement();
  auto variable = new VariableDeclaration::VariableDeclaration(
      (llvm::Type *)llvm::Type::getInt32Ty(module.getContext()), variableName);
  statement->AddStatement(variable);

  // Create function and add statement in it.
  auto function = new Function::Function(
      (llvm::Type *)llvm::Type::getInt32Ty(module.getContext()), name);
  auto body = new Function::FunctionBody(statement);
  function->addFunctionBody(body);

  compilationUnit.AddCompilationUnitItems(function);

  GenerateIR(compilationUnit, module);
}