#include "Lexer/Tokens.h"

#include <gmock/gmock-matchers.h>
#include <gmock/gmock.h>
#include <gtest/gtest.h>

#include <Lexer/Lexer.h>

// stl
#include <string>
#include <unordered_map>
#include <vector>

using namespace Lexer;

using tokenHandlingLambda = std::function<void(const Token &)>;

void getAllTokens(tokenHandlingLambda &&lambda, std::string stream) {
  Tokenizer tokenizer(stream);
  Token currentToken;
  while ((currentToken = tokenizer.getToken()).type != END_OF_FILE) {
    lambda(currentToken);
  }
}

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

  std::vector<Token> ParsedTokensTypes;
  getAllTokens(
      [&ParsedTokensTypes](const Token currentToken) {
        ParsedTokensTypes.push_back(currentToken);
      },
      stream);

  // TODO make custom matcher for this, the test failure is horrible for this..
  // With the custom operator== printer in Token the error can be read at the
  // top and which element it pertains to at the bottom.
  // clang-format off
  EXPECT_THAT(ParsedTokensTypes,
              testing::ElementsAre(
                  Token{NEWLINE}, Token{NEWLINE},

                  /* int main() [] { */
                  Token{INT}, Token{IDENTIFER, "main"}, Token{LEFT_PARENTHESIS}, Token{RIGHT_PARENTHESIS},
                  Token{LEFT_BRACKET}, Token{RIGHT_BRACKET}, Token{LEFT_CURLY_BRACE}, Token{NEWLINE}, 
                  
                  /* int a = "ab\"c" */
                  Token{INT}, Token{IDENTIFER, "a"}, Token{EQUAL}, Token{STRING_LITERAL, R"(ab"c)"}, Token{NEWLINE}, 
                  
                  /* if(1) { */
                  Token{IF}, Token{LEFT_PARENTHESIS}, Token{INTEGER_LITERAL, "1"}, Token{RIGHT_PARENTHESIS},
                  Token{LEFT_CURLY_BRACE}, Token{NEWLINE}, 
                  
                  /*  string b */
                  Token{STRING}, Token{IDENTIFER, "b"}, Token{NEWLINE}, 

                  /* } */
                  Token{RIGHT_CURLY_BRACE}, Token{NEWLINE}, 
                  
                  /* else { */
                  Token{ELSE}, Token{LEFT_CURLY_BRACE}, Token{NEWLINE}, 
                  
                  /* string a */
                  Token{STRING}, Token{IDENTIFER, "a"}, Token{NEWLINE}, 

                  /* } */
                  Token{RIGHT_CURLY_BRACE}, Token{NEWLINE}, Token{NEWLINE}, 
                  
                  /* return 1*/
                  Token{RETURN}, Token{INTEGER_LITERAL, "1"}, Token{NEWLINE}, 

                  /* } */
                  Token{RIGHT_CURLY_BRACE}, Token{NEWLINE}, Token{NEWLINE}));
  // clang-format on
}

// Expect that all spaces are skipped
TEST(Lexer, TestSpaces) {

  // All spaces
  std::string stream = R"(                        )";
  stream += EOF;

  std::vector<Token> ParsedTokensTypes;
  getAllTokens(
      [&ParsedTokensTypes](const Token currentToken) {
        ParsedTokensTypes.push_back(currentToken);
      },
      stream);

  EXPECT_TRUE(ParsedTokensTypes.empty());

  // Spaces mixed with other stuff
  stream = R"( a    b    ddd     int   1  
  
    if(1) {
      string b
    }
    else {
      string a
    }
  
  )";
  stream += EOF;

  std::unordered_map<std::string, int> ParsedTokensValues;
  getAllTokens(
      [&ParsedTokensValues](const Token currentToken) {
        if (ParsedTokensValues.contains(currentToken.value)) {
          ParsedTokensValues.at(currentToken.value)++;
        } else {
          ParsedTokensValues.emplace(currentToken.value, 1);
        }
      },
      stream);

  // Cant check if a space token comes out, because there isn't one.
  // But check that the value of any token that come out are not equal to space.
  EXPECT_FALSE(ParsedTokensValues.contains(" "));
}

/// Test that the expected number of newlines can be found.
TEST(Lexer, TestNewLines) {
  std::string stream = R"(
  int aa

  string bb

  cc
  )";
  stream += EOF;

  int numberOfNewlines = 0;
  getAllTokens(
      [&numberOfNewlines](const Token currentToken) {
        if (currentToken.type == Lexer::NEWLINE) {
          ++numberOfNewlines;
          EXPECT_TRUE(currentToken.value.empty());
        }
      },
      stream);

  EXPECT_EQ(numberOfNewlines, 6);
}
TEST(Lexer, TestGetReservedOrIdentifier) {
  std::string stream =
      R"(if else elif aa return or and asd int sint32 uint32 ooOoo bool string char BruHH oMg )";
  stream += EOF;

  int numberOfReserved = 0;
  int numberOfIdentifier = 0;
  getAllTokens(
      [&numberOfReserved, &numberOfIdentifier](const Token currentToken) {
        if (currentToken.type == TokenType::IDENTIFER) {
          // Check that identifier value is in the token.
          EXPECT_FALSE(currentToken.value.empty());
          EXPECT_FALSE(reserverdTypeToToken.contains(currentToken.value));
          ++numberOfIdentifier;
        } else {
          EXPECT_TRUE(currentToken.value.empty());

          // Check that the token type can be found in the ParsedTokensTypes map
          int numberOfFoundValues = 0;
          for (auto [_, type] : reserverdTypeToToken) {
            if (type == currentToken.type) {
              ++numberOfFoundValues;
            }
          }
          EXPECT_EQ(numberOfFoundValues, 1);
          ++numberOfReserved;
        }
      },
      stream);

      EXPECT_EQ(numberOfReserved, 12);
      EXPECT_EQ(numberOfIdentifier, 5);
}

// TODO add test for getNumber, skipComment, getSeparatorOrOperatorToken, getEndOfFile, getUnknown