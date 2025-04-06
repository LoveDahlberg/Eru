// stl
#include <cassert>
#include <cctype>
#include <string>

// include
#include <Support/Constants.h>
#include <Lexer/Tokens.h>
#include <Lexer/Lexer.h>

using namespace Lexing;

// TODO, get next and lookahead could be an object used by the lexer
// It could be attached to the 'input' object itself.
int Lexer::getNext() {
  if (index >= input.size()) {
    return EOF;
  }
  assert(index + 1 > index && "index is overflowing");

  return input[++index];
}

int Lexer::getCurrent() {
  if (index >= input.size()) {
    return EOF;
  }

  return input[index];
}

int Lexer::lookAhead() {
  // What to do here?
  if (index >= input.size()) {
    return EOF;
  }
  assert(index + 1 > index && "index is overflowing");

  return input[index + 1];
}

// TODO all getters could be could be part of a token builder object.
Token Lexer::getReservedOrIdentifier() {
  auto token = Token();

  // TODO make this accept identifers with special characters that are
  // acceptable, like _
  token.value = currentChar;
  while (isalnum((currentChar = getNext()))) {
    token.value += currentChar;
  }

  if (reserverdTypeToToken.contains(token.value)) {
    token.type = reserverdTypeToToken.at(token.value);
    token.value = "";
  } else {
    token.type = TokenType::IDENTIFER;
  }
  return token;
}

Token Lexer::getNumber() {
  auto token = Token();
  token.type = TokenType::INTEGER_LITERAL;

  // TODO add loopLimit
  do {
    token.value += currentChar;
    currentChar = getNext();
  } while (isdigit(currentChar) || currentChar == '.');

  // TODO if you pass 1abc it will take the 1 and read abc after.
  // It should return an unknown token here when it is an invalid number.

  return token;
}

void Lexer::skipComment() {
  if (lookAhead() == '*') {
    currentChar = getNext();
    while (currentChar != EOF) {
      if (currentChar == '*' && lookAhead() == '/') {
        currentChar = getNext();
        break;
      }
      currentChar = getNext();
    }
  } else if (lookAhead() == '/') {
    do
      currentChar = getNext();
    while (currentChar != EOF && currentChar != '\n' && currentChar != '\r');
  } else {
    currentChar = getNext();
  }
}

Token Lexer::getStringLiteral() {
  Token token;
  token.type = TokenType::STRING_LITERAL;

  // TODO add loopLimit
  bool escapeCharacter;
  do {
    escapeCharacter = false;
    currentChar = getNext();

    if (currentChar == '\\') {
      escapeCharacter = true;
      token.value += getNext();
      continue;
    }

    if (currentChar == '"') {
      continue;
    }

    // Improve to disallow problematic things here..
    token.value += currentChar;
  } while (escapeCharacter || currentChar != '"');

  currentChar = getNext();
  return token;
}

bool Lexer::isSeparatorOrOperatorToken() {
  return separatorOperatorToToken.contains(currentChar);
}

Token Lexer::getSeparatorOrOperatorToken() {
  Token token;
  token.type = separatorOperatorToToken.at(currentChar);
  currentChar = getNext();
  return token;
}

Token Lexer::getNewline() {
  Token token;
  token.type = TokenType::NEWLINE;

  currentChar = getNext();
  return token;
}

Token Lexer::getEndOfFile() {
  Token token;
  token.type = TokenType::END_OF_FILE;
  return token;
}

Token Lexer::getUnknown() {
  Token token;
  token.value = currentChar;
  currentChar = getNext();
  return token;
}

bool isNewline(int currentChar) {
  return currentChar == '\n' || currentChar == '\r';
}

Token Lexer::getNextToken() {
  currentChar = getCurrent();
  while (isspace(currentChar)) {
    if (isNewline(currentChar)) {
      return getNewline();
    }
    currentChar = getNext();
  }

  if (isalpha(currentChar)) {
    return getReservedOrIdentifier();
  }

  if (isdigit(currentChar)) {
    return getNumber();
  }

  if (currentChar == '"') {
    return getStringLiteral();
  }

  if (currentChar == '/') {
    skipComment();
    return getNextToken();
  }

  if (isSeparatorOrOperatorToken()) {
    return getSeparatorOrOperatorToken();
  }

  if (currentChar == EOF) {
    return getEndOfFile();
  }

  // What case is this?
  return getUnknown();
}
