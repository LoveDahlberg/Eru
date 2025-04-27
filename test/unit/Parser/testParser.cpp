#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <Lexer/Lexer.h>
#include <Lexer/Tokens.h>
#include <Parser/Parser.h>

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
  auto parserItems = ParseCompilationUnit(lexer);

  EXPECT_EQ(parserItems.compilationUnit.GetAddCompilationUnitItems().size(), 5);
}