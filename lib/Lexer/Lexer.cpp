// stdlib
#include <cctype>

#include <Lexer/Lexer.h>

int getNextCharacter(std::string input) {
  static size_t index = 0;
  if (index >= input.size()) {
    return EOF;
  }
  return input[index++];
}

int lookAhead(std::string input) {}

Token getToken(std::string input) {
  int LastChar = ' ';
  auto token = Token();

  while (isspace(LastChar)) {
    LastChar = getNextCharacter(input);
    if (isalpha(LastChar)) { // identifier: [a-zA-Z][a-zA-Z0-9]*
      token.identifier = LastChar;
      while (isalnum((LastChar = getNextCharacter(input)))) {
        token.identifier += LastChar;
      }
      // Figure out if the characters form an identifier.
      return token;
    }

    if (isdigit(LastChar) || LastChar == '.') { // Number: [0-9.]+
      std::string NumStr;
      do {
        NumStr += LastChar;
        LastChar = getNextCharacter(input);
      } while (isdigit(LastChar) || LastChar == '.');

      token.number = strtod(NumStr.c_str(), nullptr);
      return token;
    }

    if (LastChar == '/') {
      if (lookAhead(input) == '*') {
        LastChar = getNextCharacter(input);
        while (LastChar != EOF) {
          if (LastChar == '*' && lookAhead(input) == '/') {
            LastChar = getNextCharacter(input);
            break;
          }
          LastChar = getNextCharacter(input);
        }
      } else if (lookAhead(input) == '/') { // Comment until end of line.
        do
          LastChar = getNextCharacter(input);
        while (LastChar != EOF && LastChar != '\n' && LastChar != '\r');
      }
      return getToken(input);
    }

    // Check for end of file.  Don't eat the EOF.
    if (LastChar == EOF) {
      token.type = endOfFile;
      return token;
    }

    // Otherwise, just return the character as its ascii value.
    token.rawValue = LastChar;
    // \LastChar = getNextCharacter(input);
    return token;
  }
}