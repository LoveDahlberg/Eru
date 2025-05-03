#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <Lexer/Lexer.h>
#include <Lexer/Tokens.h>
#include <Parser/Syntax/CompilationUnit.h>

using namespace Parser;

TEST(Parser, TestDeclarations) {
  std::string stream = R"(
    int first
    string second

    int fourth()
    int bruh(char one)
    uint32 brotha(sint32 hello, bool world)
    )";
  stream += EOF;

  Lexer lexer(stream);
  auto parserItems = Syntax::ParseCompilationUnit(lexer);

  ASSERT_TRUE(parserItems);

  EXPECT_EQ((*parserItems).compilationUnit.GetAddCompilationUnitItems().size(), 5);
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
      }
      else
      {
        int bb
        alsoSomething(a, bb, cc)
      }
    }
    )";
  stream += EOF;

  Lexer lexer(stream);
  auto parserItems = Syntax::ParseCompilationUnit(lexer);

  ASSERT_TRUE(parserItems);
}