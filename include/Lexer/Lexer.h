#pragma once

#include <filesystem>
#include <string>

#include <Support/IO/FileReader.h>

#include <Lexer/Tokens.h>

namespace Lexer {

struct Token {
  TokenType type = TokenType::NONE;
  std::string value;

  bool operator==(const Token&) const = default;
};

class Tokenizer {
public:
  Tokenizer(const std::string &input) : input(input) {}

  Token getToken();

private:
  int lookAhead();
  int getNext();

  Token getReservedOrIdentifier();
  Token getNumber();
  void skipComment();
  Token getStringLiteral();
  bool isSeparatorOrOperatorToken();
  Token getSeparatorOrOperatorToken();

private:
  const std::string input;
  long long unsigned index = 0;
  int currentChar = ' ';
};

} // namespace Lexer
