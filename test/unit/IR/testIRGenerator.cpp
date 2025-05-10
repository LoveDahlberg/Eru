// include
#include "AST/Assignment.h"
#include "AST/Controlflow.h"
#include "AST/Expression.h"
#include <AST/Function.h>
#include <AST/Statement.h>
#include <AST/Types.h>
#include <AST/VariableDeclaration.h>
#include <IR/IRGenerator.h>

// llvm
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>

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

  EXPECT_TRUE(llvm::verifyModule(module));
}

Statement::Statement *CreateTestFunctionAndGetInsideStmnt(llvm::Module &module,
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

  auto statement = CreateTestFunctionAndGetInsideStmnt(module, compilationUnit);

  auto cuItems = GenerateIR(compilationUnit, module);
  EXPECT_THAT(cuItems, testing::Each(testing::NotNull()));

  EXPECT_TRUE(llvm::verifyModule(module));
}

TEST(IR, testFunctionVariable) {
  auto ctx = new llvm::LLVMContext();
  auto module = llvm::Module("", *ctx);
  CompilationUnit compilationUnit;

  auto statement = CreateTestFunctionAndGetInsideStmnt(module, compilationUnit);

  constexpr const char *variableName = "firstVariable";
  auto variable = new VariableDeclaration::VariableDeclaration(
      (llvm::Type *)llvm::Type::getInt32Ty(module.getContext()), variableName);
  statement->AddStatement(variable);

  auto cuItems = GenerateIR(compilationUnit, module);
  EXPECT_THAT(cuItems, testing::Each(testing::NotNull()));

  EXPECT_TRUE(llvm::verifyModule(module));
}

TEST(IR, testDeclarationAssignment) {
  auto ctx = new llvm::LLVMContext();
  auto module = llvm::Module("", *ctx);
  CompilationUnit compilationUnit;

  auto statement = CreateTestFunctionAndGetInsideStmnt(module, compilationUnit);

  constexpr const char *variableName = "firstVariable";
  auto variable = new VariableDeclaration::VariableDeclaration(
      (llvm::Type *)llvm::Type::getInt32Ty(module.getContext()), variableName);

  auto assignment = new Assignment::Assignment(variable);

  auto expression = new Expression::Expression();
  auto unit1 =
      new Expression::ExpressionUnit(std::nullopt, Types::IntegerLiteral("1"));
  expression->addExpressionUnit(unit1);

  auto unit2 = new Expression::ExpressionUnit(
      Expression::ArithmeticOperator::PLUS, Types::IntegerLiteral("2"));
  expression->addExpressionUnit(unit2);

  auto unit3 = new Expression::ExpressionUnit(
      Expression::ArithmeticOperator::PLUS, Types::IntegerLiteral("6"));
  expression->addExpressionUnit(unit3);

  auto unit4 = new Expression::ExpressionUnit(
      Expression::ArithmeticOperator::MINUS, Types::IntegerLiteral("4"));
  expression->addExpressionUnit(unit4);

  assignment->setExpression(&expression);

  statement->AddStatement(assignment);

  auto cuItems = GenerateIR(compilationUnit, module);
  EXPECT_THAT(cuItems, testing::Each(testing::NotNull()));
  EXPECT_TRUE(llvm::verifyModule(module));
}

TEST(IR, testFunctionCall) {
  auto ctx = new llvm::LLVMContext();
  auto module = llvm::Module("", *ctx);
  CompilationUnit compilationUnit;

  constexpr const auto functionToCall = "functionToCall";
  constexpr const auto parameterName = "firstParameter";

  auto parametersDeclaration = new VariableDeclaration::Variable(
      (llvm::Type *)llvm::Type::getInt32Ty(module.getContext()), parameterName);
  auto functionDeclaration = new Function::Function(
      (llvm::Type *)llvm::Type::getInt32Ty(module.getContext()), functionToCall,
      {parametersDeclaration});
  compilationUnit.AddCompilationUnitItems(functionDeclaration);

  auto statement = CreateTestFunctionAndGetInsideStmnt(module, compilationUnit);

  auto expression = new Expression::Expression();
  auto unit1 =
      new Expression::ExpressionUnit(std::nullopt, Types::IntegerLiteral("1"));
  expression->addExpressionUnit(unit1);
  auto call = new Function::FunctionCall(functionToCall, {expression});

  statement->AddStatement(call);

  auto cuItems = GenerateIR(compilationUnit, module);
  EXPECT_THAT(cuItems, testing::Each(testing::NotNull()));
  EXPECT_TRUE(llvm::verifyModule(module));
}

TEST(IR, testConditionalBranch) {
  auto ctx = new llvm::LLVMContext();
  auto module = llvm::Module("", *ctx);
  CompilationUnit compilationUnit;

  auto statement = CreateTestFunctionAndGetInsideStmnt(module, compilationUnit);

  std::vector<Controlflow::ConditionalBranch *> conditionalChain;

  // The first if condition
  auto expressionIf = new Expression::Expression();
  auto unit1 =
      new Expression::ExpressionUnit(std::nullopt, Types::IntegerLiteral("1"));
  expressionIf->addExpressionUnit(unit1);

  // The body of the first if statement
  auto statementIf = new Statement::Statement();
  constexpr const char *variableNameIf = "ifVariable";
  auto variableIf = new VariableDeclaration::VariableDeclaration(
      (llvm::Type *)llvm::Type::getInt32Ty(module.getContext()),
      variableNameIf);
  statementIf->AddStatement(variableIf);

  auto branchIf =
      new Controlflow::ConditionalBranch(&expressionIf, &statementIf);
  conditionalChain.push_back(branchIf);

  // The elif condition
  auto expressionElif = new Expression::Expression();
  expressionElif->addExpressionUnit(unit1);

  // The body of the elif statement
  auto statementElif = new Statement::Statement();
  statementElif->AddStatement(variableIf);

  auto branchElif =
      new Controlflow::ConditionalBranch(&expressionElif, &statementElif);
  conditionalChain.push_back(branchElif);

  // The else condition
  // The body of the else statement
  auto statementElse = new Statement::Statement();
  statementElse->AddStatement(variableIf);

  auto branchElse = new Controlflow::ConditionalBranch();
  branchElse->addStatement(&statementElse);
  conditionalChain.push_back(branchElse);

  auto branch = new Controlflow::ConditionalBranchingGroup(conditionalChain);

  statement->AddStatement(branch);

  auto cuItems = GenerateIR(compilationUnit, module);
  EXPECT_THAT(cuItems, testing::Each(testing::NotNull()));
  EXPECT_TRUE(llvm::verifyModule(module));
}