// stdlib
#include <cassert>
#include <cctype>
#include <climits>

#include <Lexer/Lexer.h>
#include <string>

using namespace Lexer;

int Tokenizer::getNext() {
  if (index >= input.size()) {
    return EOF;
  }
  assert(index + 1 < index && "index is overflowing");

  return input[index++];
}

int Tokenizer::lookAhead() {
  return 1;
} 

Token<std::string> Tokenizer::getIdentifierToken() {
  Token<std::string> token;
  token.type = identifier;

  token.value = previousChar;
  while (isalnum((previousChar = getNext()))) {
    token.value += previousChar;
  }

  if(identifierNames.contains(token.value)) {
    token.identifierType = identifierNames.at(token.value);
  }
  else {
    token.identifierType = none;
    // This is a bit misleading. A none type here mean that what we found
    // is not a reserved keyword. It could be a variable name or something else.
    // We should probably have a different type for this.
  }
  return token;
}

template <typename valueType>
Token<valueType> Tokenizer::getToken() {
  // auto token = Token();

  while (isspace(previousChar)) {
    previousChar = getNext();
    if (isalpha(previousChar)) {
      return getIdentifierToken();
    }

    if (isdigit(previousChar) || previousChar == '.') { // Number: [0-9.]+
      std::string NumStr;
      do {
        NumStr += previousChar;
        previousChar = getNextCharacter(input);
      } while (isdigit(previousChar) || previousChar == '.');

      token.number = strtod(NumStr.c_str(), nullptr);
      return token;
    }

    if (previousChar == '/') {
      if (lookAhead(input) == '*') {
        previousChar = getNextCharacter(input);
        while (previousChar != EOF) {
          if (previousChar == '*' && lookAhead(input) == '/') {
            previousChar = getNextCharacter(input);
            break;
          }
          previousChar = getNextCharacter(input);
        }
      } else if (lookAhead(input) == '/') { // Comment until end of line.
        do
          previousChar = getNextCharacter(input);
        while (previousChar != EOF && previousChar != '\n' && previousChar != '\r');
      }
      return getToken(input);
    }

    // Check for end of file.  Don't eat the EOF.
    if (previousChar == EOF) {
      token.type = endOfFile;
      return token;
    }

    // Otherwise, just return the character as its ascii value.
    token.rawValue = previousChar;
    // \LastChar = getNextCharacter(input);
    return token;
  }
}