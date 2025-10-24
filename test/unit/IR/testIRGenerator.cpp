// include
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
      AST::Types::Types::INT, variableName);
  variable->isGlobal = true;

  auto initializer =
      new VariableDeclaration::GlobalVariableInitialization(variable);
  initializer->constOperand =
      AST::Expression::ConstantOperand(AST::Types::IntegerLiteral("1"));

  compilationUnit.AddCompilationUnitItems(initializer);

  auto generator = IRGenerator(module);
  auto context = Context::ASTContext(&compilationUnit);

  EXPECT_THAT(generator.Walk(context), testing::Each(testing::NotNull()));

  auto generatedVariable = module.getGlobalVariable(variableName, true);
  EXPECT_NE(generatedVariable, nullptr);

  EXPECT_FALSE(llvm::verifyModule(module, &llvm::errs()));
}

Expression::Expression *
CreateTestExpression(std::optional<Lexing::Operator> opreator,
                     ::AST::Expression::Operand operand) {
  auto expression = new Expression::Expression();
  auto unit = new Expression::ExpressionUnit(opreator, operand);
  expression->addExpressionUnit(unit);
  return expression;
}

Statement::Statement *CreateTestFunctionAndGetInsideStmnt(llvm::Module &module,
                                                          CompilationUnit &cu) {
  constexpr const char *name = "function";

  // Create statement.
  auto statement = new Statement::Statement();

  // Create block
  auto block = new Function::Block(
      statement,
      CreateTestExpression(std::nullopt, Types::IntegerLiteral("1")));

  // Create function and add a block into it.
  auto function = new Function::Function(Types::Types::INT, name);
  auto body = new Function::FunctionBody(block);
  function->addFunctionBody(body);

  cu.AddCompilationUnitItems(function);

  return statement;
}

TEST(IR, testFunction) {
  auto ctx = new llvm::LLVMContext();
  auto module = llvm::Module("", *ctx);
  CompilationUnit compilationUnit;

  auto statement = CreateTestFunctionAndGetInsideStmnt(module, compilationUnit);

  auto generator = IRGenerator(module);
  auto context = Context::ASTContext(&compilationUnit);

  EXPECT_THAT(generator.Walk(context), testing::Each(testing::NotNull()));

  EXPECT_FALSE(llvm::verifyModule(module, &llvm::errs()));
}

TEST(IR, testFunctionVariable) {
  auto ctx = new llvm::LLVMContext();
  auto module = llvm::Module("", *ctx);
  CompilationUnit compilationUnit;

  auto statement = CreateTestFunctionAndGetInsideStmnt(module, compilationUnit);

  constexpr const char *variableName = "firstVariable";
  auto variable = new VariableDeclaration::VariableDeclaration(
      AST::Types::Types::INT, variableName);
  statement->AddStatement(variable);

  auto generator = IRGenerator(module);
  auto context = Context::ASTContext(&compilationUnit);

  EXPECT_THAT(generator.Walk(context), testing::Each(testing::NotNull()));

  EXPECT_FALSE(llvm::verifyModule(module, &llvm::errs()));
}

TEST(IR, testDeclarationAssignment) {
  auto ctx = new llvm::LLVMContext();
  auto module = llvm::Module("", *ctx);
  CompilationUnit compilationUnit;

  auto statement = CreateTestFunctionAndGetInsideStmnt(module, compilationUnit);

  constexpr const char *variableName = "firstVariable";
  auto variable = new VariableDeclaration::VariableDeclaration(
      AST::Types::Types::INT, variableName);

  auto assignment = new Assignment::Assignment(variable);

  auto expression = new Expression::Expression();
  auto unit1 =
      new Expression::ExpressionUnit(std::nullopt, Types::IntegerLiteral("1"));
  expression->addExpressionUnit(unit1);

  auto unit2 = new Expression::ExpressionUnit(Lexing::Operator::PLUS,
                                              Types::IntegerLiteral("2"));
  expression->addExpressionUnit(unit2);

  auto unit3 = new Expression::ExpressionUnit(Lexing::Operator::PLUS,
                                              Types::IntegerLiteral("6"));
  expression->addExpressionUnit(unit3);

  auto unit4 = new Expression::ExpressionUnit(Lexing::Operator::MINUS,
                                              Types::IntegerLiteral("4"));
  expression->addExpressionUnit(unit4);

  assignment->setExpression(&expression);

  statement->AddStatement(assignment);

  auto generator = IRGenerator(module);
  auto context = Context::ASTContext(&compilationUnit);

  EXPECT_THAT(generator.Walk(context), testing::Each(testing::NotNull()));
  EXPECT_FALSE(llvm::verifyModule(module, &llvm::errs()));
}

TEST(IR, testFunctionCall) {
  auto ctx = new llvm::LLVMContext();
  auto module = llvm::Module("", *ctx);
  CompilationUnit compilationUnit;

  constexpr const auto functionToCall = "functionToCall";
  constexpr const auto parameterName = "firstParameter";

  auto parametersDeclaration =
      new VariableDeclaration::Variable(AST::Types::Types::INT, parameterName);
  auto functionDeclaration = new Function::Function(
      AST::Types::Types::INT, functionToCall, {parametersDeclaration});
  compilationUnit.AddCompilationUnitItems(functionDeclaration);

  auto statement = CreateTestFunctionAndGetInsideStmnt(module, compilationUnit);

  auto call = new Function::FunctionCall(
      functionToCall,
      {CreateTestExpression(std::nullopt, Types::IntegerLiteral("1"))});

  statement->AddStatement(call);

  auto generator = IRGenerator(module);
  auto context = Context::ASTContext(&compilationUnit);

  EXPECT_THAT(generator.Walk(context), testing::Each(testing::NotNull()));
  EXPECT_FALSE(llvm::verifyModule(module, &llvm::errs()));
}

Controlflow::ConditionalBranchingGroup *
CreateSingleTestConditionalBranch(llvm::Module &module,
                                  bool hasReturnValue = true) {
  std::vector<Controlflow::ConditionalBranch *> conditionalChain;
  // The first if condition
  auto expressionIf =
      CreateTestExpression(std::nullopt, Types::IntegerLiteral("1"));

  // The body of the first if statement
  auto statementIf = new Statement::Statement();
  auto variableIf = new VariableDeclaration::VariableDeclaration(
      AST::Types::Types::INT, "testIfVariable");
  statementIf->AddStatement(variableIf);

  auto blockIf = new Function::Block(
      statementIf,
      hasReturnValue
          ? CreateTestExpression(std::nullopt, Types::IntegerLiteral("42"))
          : nullptr);

  auto branchIf = new Controlflow::ConditionalBranch(&expressionIf, &blockIf);
  conditionalChain.push_back(branchIf);
  return new Controlflow::ConditionalBranchingGroup(conditionalChain);
}

TEST(IR, testConditionalBranch) {
  auto ctx = new llvm::LLVMContext();
  auto module = llvm::Module("", *ctx);
  CompilationUnit compilationUnit;

  auto statement = CreateTestFunctionAndGetInsideStmnt(module, compilationUnit);

  std::vector<Controlflow::ConditionalBranch *> conditionalChain;

  // The first if condition
  auto expressionIf =
      CreateTestExpression(std::nullopt, Types::IntegerLiteral("1"));

  // The body of the first if statement
  auto statementIf = new Statement::Statement();
  auto variableIf = new VariableDeclaration::VariableDeclaration(
      AST::Types::Types::INT, "ifVariable");
  statementIf->AddStatement(variableIf);

  auto blockIf = new Function::Block(
      statementIf,
      CreateTestExpression(std::nullopt, Types::IntegerLiteral("1")));

  auto branchIf = new Controlflow::ConditionalBranch(&expressionIf, &blockIf);
  conditionalChain.push_back(branchIf);

  // The elif condition
  auto expressionElif =
      CreateTestExpression(std::nullopt, Types::IntegerLiteral("2"));

  // The body of the elif statement
  auto statementElif = new Statement::Statement();
  statementElif->AddStatement(variableIf);
  statementElif->AddStatement(CreateSingleTestConditionalBranch(module));

  auto blockElIf = new Function::Block(statementElif, nullptr);

  auto branchElif =
      new Controlflow::ConditionalBranch(&expressionElif, &blockElIf);
  conditionalChain.push_back(branchElif);

  // The else condition
  // The body of the else statement
  auto statementElse = new Statement::Statement();
  statementElse->AddStatement(variableIf);

  auto blockElse = new Function::Block(
      statementElse,
      CreateTestExpression(std::nullopt, Types::IntegerLiteral("3")));

  auto branchElse = new Controlflow::ConditionalBranch();
  branchElse->addBlock(&blockElse);
  conditionalChain.push_back(branchElse);

  auto branch = new Controlflow::ConditionalBranchingGroup(conditionalChain);

  statement->AddStatement(branch);

  auto generator = IRGenerator(module);
  auto context = Context::ASTContext(&compilationUnit);

  EXPECT_THAT(generator.Walk(context), testing::Each(testing::NotNull()));
  EXPECT_FALSE(llvm::verifyModule(module, &llvm::errs()));
}