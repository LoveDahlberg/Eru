// include
#include "AST/Assignment.h"
#include "AST/Expression.h"
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

TEST(IR, testGlobalVarialbe) {

  auto ctx = new llvm::LLVMContext();
  auto module = llvm::Module("", *ctx);
  CompilationUnit compilationUnit;

  constexpr const char *variableName = "firstVariable";
  auto variable = new VariableDeclaration::VariableDeclaration(
      (llvm::Type *)llvm::Type::getInt32Ty(module.getContext()), variableName);
  variable->global = true;
  compilationUnit.AddCompilationUnitItems(variable);

  auto cuItems = GenerateIR(compilationUnit, module);
  EXPECT_THAT(cuItems, testing::Each(testing::NotNull()));

  auto generatedVariable = module.getGlobalVariable(variableName, true);
  EXPECT_NE(generatedVariable, nullptr);
}

Statement::Statement *CreateFunctionAndGetInsideStmnt(llvm::Module &module,
                                                      CompilationUnit &cu) {
  constexpr const char *name = "function";

  // Create statement.
  auto statement = new Statement::Statement();

  // Create function and add statement in it.
  auto function = new Function::Function(
      (llvm::Type *)llvm::Type::getInt32Ty(module.getContext()), name);
  auto body = new Function::FunctionBody(statement);
  function->addFunctionBody(body);

  cu.AddCompilationUnitItems(function);

  return statement;
}

TEST(IR, testFunction) {
  auto ctx = new llvm::LLVMContext();
  auto module = llvm::Module("", *ctx);
  CompilationUnit compilationUnit;

  auto statement = CreateFunctionAndGetInsideStmnt(module, compilationUnit);

  // constexpr const char *variableName = "firstVariable";
  // auto variable = new VariableDeclaration::VariableDeclaration(
  //     (llvm::Type *)llvm::Type::getInt32Ty(module.getContext()),
  //     variableName);
  // statement->AddStatement(variable);

  auto cuItems = GenerateIR(compilationUnit, module);
  EXPECT_THAT(cuItems, testing::Each(testing::NotNull()));
}

TEST(IR, testFunctionVariable) {
  auto ctx = new llvm::LLVMContext();
  auto module = llvm::Module("", *ctx);
  CompilationUnit compilationUnit;

  auto statement = CreateFunctionAndGetInsideStmnt(module, compilationUnit);

  constexpr const char *variableName = "firstVariable";
  auto variable = new VariableDeclaration::VariableDeclaration(
      (llvm::Type *)llvm::Type::getInt32Ty(module.getContext()), variableName);
  statement->AddStatement(variable);

  auto cuItems = GenerateIR(compilationUnit, module);
  EXPECT_THAT(cuItems, testing::Each(testing::NotNull()));
}

TEST(IR, testDeclarationAssignment) {
  auto ctx = new llvm::LLVMContext();
  auto module = llvm::Module("", *ctx);
  CompilationUnit compilationUnit;

  auto statement = CreateFunctionAndGetInsideStmnt(module, compilationUnit);

  constexpr const char *variableName = "firstVariable";
  auto variable = new VariableDeclaration::VariableDeclaration(
      (llvm::Type *)llvm::Type::getInt32Ty(module.getContext()), variableName);

  auto assignment = new Assignment::Assignment(variable);

  auto expression = new Expression::Expression();
  auto unit1 = new Expression::ExpressionUnit(
      Types::IntegerLiteral("1"), Expression::ArithmeticOperator::PLUS);
  expression->addExpressionUnit(unit1);

  auto unit2 = new Expression::ExpressionUnit(Types::IntegerLiteral("2"));
  expression->addExpressionUnit(unit2);

  assignment->setExpression(&expression);

  statement->AddStatement(assignment);

  auto cuItems = GenerateIR(compilationUnit, module);
  EXPECT_THAT(cuItems, testing::Each(testing::NotNull()));
}