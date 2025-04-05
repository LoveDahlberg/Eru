#include "Lexer/Tokens.h"

#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <Lexer/Lexer.h>
#include <string>
#include <vector>

using namespace Lexer;

TEST(Lexer, BasicTokenization) {
  std::string stream = R"(
  
  // Comment
  int main() [] {
    int a = 1
    if(1) {
      string b
    }
    else {
      string a
    }

    return 1
  }

  )";
  stream += EOF;

  Tokenizer tokenizer(stream);

  std::vector<TokenType> ParsedTokens;
  Token currentToken;
  while ((currentToken = tokenizer.getToken()).type != END_OF_FILE) {
    ParsedTokens.push_back(currentToken.type);
  }

  // clang-format off
  EXPECT_THAT(ParsedTokens,
              testing::ElementsAre(
                  /* int main() [] { */
                  INT, IDENTIFER, LEFT_PARENTHESIS, RIGHT_PARENTHESIS, LEFT_BRACKET, RIGHT_BRACKET, LEFT_CURLY_BRACE, 
                  
                  /* int a = 1 */
                  INT, IDENTIFER, EQUAL, INTEGER_LITERAL,
                  
                  /* if(1) { */
                  IF, LEFT_PARENTHESIS, INTEGER_LITERAL, RIGHT_PARENTHESIS, LEFT_CURLY_BRACE,
                  
                  /*  string b */
                  STRING, IDENTIFER,

                  /* } */
                  RIGHT_CURLY_BRACE,
                  
                  /* else { */
                  ELSE, LEFT_CURLY_BRACE,
                  
                  /* string a */
                  STRING, IDENTIFER,

                  /* } */
                  RIGHT_CURLY_BRACE,
                  
                  /* return 1*/
                  RETURN, INTEGER_LITERAL,

                  /* } */
                  RIGHT_CURLY_BRACE));
  // clang-format on
}

// TODO Create one test per internal getter, something smaller that tries everything that could be inside.
