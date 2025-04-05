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
    int a = "ab\"c"
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

  std::vector<Token> ParsedTokensTypes;
  std::vector<std::string> ParsedTokensTokenValues;
  Token currentToken;
  while ((currentToken = tokenizer.getToken()).type != END_OF_FILE) {
    ParsedTokensTypes.push_back(currentToken);
    // ParsedTokensTokenValues.push_back(currentToken.value);
  }

  // TODO make custom matcher for this, the test failure is horrible for this..
  // clang-format off
  EXPECT_THAT(ParsedTokensTypes,
              testing::ElementsAre(
                  /* int main() [] { */
                  Token{INT}, Token{IDENTIFER, "main"}, Token{LEFT_PARENTHESIS}, Token{RIGHT_PARENTHESIS}, Token{LEFT_BRACKET}, Token{RIGHT_BRACKET}, Token{LEFT_CURLY_BRACE}, 
                  
                  /* int a = "ab\"c" */
                  Token{INT}, Token{IDENTIFER, "a"}, Token{EQUAL}, Token{STRING_LITERAL, R"(ab"c)"},
                  
                  /* if(1) { */
                  Token{IF}, Token{LEFT_PARENTHESIS}, Token{INTEGER_LITERAL, "1"}, Token{RIGHT_PARENTHESIS}, Token{LEFT_CURLY_BRACE},
                  
                  /*  string b */
                  Token{STRING}, Token{IDENTIFER, "b"},

                  /* } */
                  Token{RIGHT_CURLY_BRACE},
                  
                  /* else { */
                  Token{ELSE}, Token{LEFT_CURLY_BRACE},
                  
                  /* string a */
                  Token{STRING}, Token{IDENTIFER, "a"},

                  /* } */
                  Token{RIGHT_CURLY_BRACE},
                  
                  /* return 1*/
                  Token{RETURN}, Token{INTEGER_LITERAL, "1"},

                  /* } */
                  Token{RIGHT_CURLY_BRACE}));
    // clang-format on
}

// TODO Create one test per internal getter, something smaller that tries
// everything that could be inside.
