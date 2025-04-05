// stdlib
#include "Lexer/Tokens.h"
#include <cassert>
#include <cctype>
#include <climits>

#include <Lexer/Lexer.h>
#include <string>

using namespace Lexer;

// TODO, get next and lookahead could be an object used by the tokenizer
// It could be attached to the 'input' object itself.
int Tokenizer::getNext() {
  if (index >= input.size()) {
    return EOF;
  }
  assert(index + 1 > index && "index is overflowing");

  return input[++index];
}

int Tokenizer::lookAhead() {
  // What to do here?
  if (index >= input.size()) {
    return EOF;
  }
  assert(index + 1 > index && "index is overflowing");

  return input[index + 1];
}

// TODO all getters could be could be part of a token builder object.
Token Tokenizer::getReservedOrIdentifier() {
  auto token = Token();

  token.value = currentChar;
  while (isalnum((currentChar = getNext()))) {
    token.value += currentChar;
  }

  if (reserverdTypeToToken.contains(token.value)) {
    token.type = reserverdTypeToToken.at(token.value);
  } else {
    token.type = TokenType::IDENTIFER;
  }
  return token;
}

Token Tokenizer::getNumber() {
  auto token = Token();
  token.type = INTEGER_LITERAL;

  do {
    token.value += currentChar;
    currentChar = getNext();
  } while (isdigit(currentChar) || currentChar == '.');

  return token;
}

void Tokenizer::skipComment() {
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

Token Tokenizer::getStringLiteral() {
  Token token;
  token.type = STRING_LITERAL;

  bool escapeCharacter;
  do {
    escapeCharacter = false;
    currentChar = getNext();
    
    if(currentChar == '\\'){
      escapeCharacter = true;
      continue;
    }

    if (currentChar == '"') {
      continue;
    }

    // Improve to disallow problematic things here..
    token.value += currentChar;
  } while (escapeCharacter || currentChar != '"');

  return token;
}

bool Tokenizer::isSeparatorOrOperatorToken() {
  return separatorOperatorToToken.contains(currentChar);
}

Token Tokenizer::getSeparatorOrOperatorToken() {
  Token token;
  token.type = separatorOperatorToToken.at(currentChar);
  currentChar = getNext();
  return token;
}

Token Tokenizer::getToken() {
  // Find next non whitespace character.
  while (isspace(currentChar)) {
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
    return getToken();
  }

  if (isSeparatorOrOperatorToken()) {
    return getSeparatorOrOperatorToken();
  }

  Token token;
  if (currentChar == EOF) {
    token.type = END_OF_FILE;
    return token;
  }

  // What case is this?
  token.value = currentChar;
  currentChar = getNext();
  return token;
}