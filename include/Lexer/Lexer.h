#pragma once

#include <iostream>
#include <string>

#include <Support/IO/FileReader.h>

#include <Lexer/Tokens.h>

namespace Lexer {

struct Token {
  TokenType type = TokenType::NONE;
  std::string value;

  // TODO make a custom matcher where this is done instead..
  bool operator==(const Token &other) const {
    bool success = true;
    if (other.value != value) {
      std::cout << std::format(
          "Token equality: value '{}' is not equal to expected value '{}'\n",
          other.value, value);
      success = false;
    }

    if (other.type != type) {
      std::cout << std::format(
          "Token equality: type '{}' is not equal to expected type '{}'\n",
          std::to_string(other.type), std::to_string(type));
      success = false;
    }
    return success;
  };
};

class Tokenizer {
public:
  Tokenizer(const std::string &input) : input(input) {}

  Token getToken();

private:
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

private:
  const std::string input;
  long long unsigned index = 0;
  int currentChar = ' ';
};

} // namespace Lexer
