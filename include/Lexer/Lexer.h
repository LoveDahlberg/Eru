#pragma once

#include <string>

#include <Support/IO/File.h>

#include <Lexer/Tokens.h>

namespace Lexing {

struct Token {
  TokenType type = TokenType::NONE;
  std::string value;

  bool operator==(const Token &other) const = default;

  bool operator==(TokenType tokenType) { return type == tokenType; }
};

typedef long long unsigned indexType;

class Lexer {
public:
  explicit Lexer(const std::string &input) : input(input) {}

  Token generateNextToken() {
    currentToken = getNextToken();
    return currentToken;
  }

  Token lookaheadToken() {
    auto startIndex = index;
    auto temporaryToken = getNextToken();
    index = startIndex;
    return temporaryToken;
  }

  Token lookaheadTokenNotNewline() {
    auto startIndex = index;
    isLookingAhead = true;
    Token temporaryToken;
    do {
      temporaryToken = getNextToken();
    } while (temporaryToken.type == TokenType::NEWLINE);
    index = startIndex;
    isLookingAhead = false;
    return temporaryToken;
  }

  // TODO return current line for debugging.
  std::string getCurrentCodeLine() { return ""; }

  Token getCurrentToken() { return currentToken; }

  indexType getCurrentIndex() { return index; }

  // Reset the lexer to the token at newIndex.
  void restartFromIndex(indexType newIndex) {

    // To reset it, go back one index and generate next token forward.

    // If zero (or negative) make sure new index is also is 0.
    if (newIndex < 1) {
      newIndex = 1;
    }
    index = newIndex - 1;

    // Only show current parsed function.
    parsedInput.clear();

    skipNext = false;

    generateNextToken();
  }

  std::string getParsedInput() { return parsedInput; }

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
  
  indexType index = 0;
  int currentChar = ' ';
  Token currentToken;
  bool isLookingAhead = false;

  std::string parsedInput;
  bool skipNext = false;
};

} // namespace Lexing
