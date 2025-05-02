#pragma once

#include <iostream>
#include <string>

#include <Support/IO/FileReader.h>

#include <Lexer/Tokens.h>

namespace Lexing {

struct Token {
  TokenType type = TokenType::NONE;
  std::string value;

  // TODO make a custom matcher where this is done instead..
  bool operator==(const Token &other) const {
    bool success = true;
    // if (other.value != value) {
    //   std::cout << std::format(
    //       "Token equality: value '{}' is not equal to expected value '{}'\n",
    //       other.value, value);
    //   success = false;
    // }

    // if (other.type != type) {
    //   std::cout << std::format(
    //       "Token equality: type '{}' is not equal to expected type '{}'\n",
    //       std::to_string(static_cast<int>(other.type)),
    //       std::to_string(static_cast<int>(type)));
    //   success = false;
    // }
    return success;
  };
};

class Lexer {
public:
  Lexer(const std::string &input) : input(input) {}

  Token generateNextToken(){
    currentToken = getNextToken();
    return currentToken;
  }

  Token lookaheadToken() {
    auto startIndex = index;
    auto temporaryToken = getNextToken();
    index = startIndex;
    return temporaryToken;
  }

  Token lookaheadTokenNotNewline()
  {
    auto startIndex = index;
    Token temporaryToken;
    do {
      temporaryToken = getNextToken();
    }while (temporaryToken.type == TokenType::NEWLINE);
    index = startIndex;
    return temporaryToken;
  }

  Token getCurrentToken(){
    return currentToken;
  }

private:
  Token getNextToken();

  int lookAhead();
  int getNext();
  int getCurrent();

  Token getReservedOrIdentifier();
  Token getNumber();
  void skipComment();
  Token getStringLiteral();
  bool isSeparatorOrOperatorToken();
  Token getSeparatorOrOperatorToken();
  Token getNewline();
  Token getEndOfFile();
  Token getUnknown();

  const std::string input;
  std::string parsedInput;
  long long unsigned index = 0;
  int currentChar = ' ';
  Token currentToken;
};

} // namespace Lexing
