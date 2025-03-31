#pragma once

#include <filesystem>
#include <string>

#include <Support/IO/FileReader.h>
#include <unordered_map>

namespace Lexer {

enum TokenType { endOfFile, identifier, number };

enum Identifier {
  none,
  integer,
  string,
  boolean,
};
using Identifiers = std::unordered_map<std::string, Identifier>;
const Identifiers identifierNames = {
    {"integer", integer}, {"string", string}, {"boolean", boolean}};

template <typename valueType> struct Token {
  TokenType type;
  valueType value;
  Identifier identifierType;
};

class Tokenizer {
public:
  Tokenizer(std::filesystem::path path) : input(getFileContent(path)) {}

  template <typename valueType> Token<valueType> getToken();

private:
  int lookAhead();
  int getNext();

  Token<std::string> getIdentifierToken();

private:
  std::string input;
  long long unsigned index = 0;
  int previousChar = ' ';
};

} // namespace Lexer
