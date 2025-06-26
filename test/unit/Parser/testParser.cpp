#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <Lexer/Lexer.h>
#include <Lexer/Tokens.h>

#include <Parser/Parser.h>

TEST(Parser, TestDeclarations) {
  std::string stream = R"(
    int first
    char second

    int fourth()
    int bruh(char one)
    uint32 brotha(sint32 hello, bool world)
    )";
  stream += EOF;

  Lexing::Lexer lexer(stream);

  AST::Context::ASTContext astContext;
  Analyzer::Analyzer analyzer(astContext);

  Parser::Parser parser(astContext, analyzer, lexer);

  auto parserItems = parser.Parse();

  if (parserItems.hasFailed) {
    std::cout << "\nTestFunctions failed:\n";
    for (auto reason : parserItems.failureReasons) {
      std::cout << reason << "\n";
    }
    std::cout << "\n\n";
  }
  ASSERT_FALSE(parserItems.hasFailed);

  EXPECT_EQ(astContext.compilationUnit->compilationUnitItems.size(), 5);
}

TEST(Parser, TestFunctions) {
  std::string stream = R"(
    int function(int a, bool b) [] {
      int first = 3 - "asd"
      bool second = 2 + a

      char cc = third()
      if(1 + 2) {
        int aa
        something(aa, b)
      }
      elif(second){
        somethingElse(first)
        return 2
      }
      else
      {
        int bb
        alsoSomething(a, bb, cc)
      }
      return 1
    }
    )";
  stream += EOF;

  Lexer lexer(stream);

  AST::Context::ASTContext astContext;
  Analyzer::Analyzer analyzer(astContext);

  Parser::Parser parser(astContext, analyzer, lexer);

  auto parserItems = parser.Parse();

  if (parserItems.hasFailed) {
    std::cout << "\nTestFunctions failed:\n";
    for (auto reason : parserItems.failureReasons) {
      std::cout << reason << "\n";
    }
    std::cout << "\n\n";
  }

  ASSERT_FALSE(parserItems.hasFailed);
}

// TODO add more tests for each sub category.