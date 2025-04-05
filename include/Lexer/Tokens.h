#pragma once

#include <unordered_map>
#include <string>

namespace Lexer {

enum TokenType {
  NONE,

  //  Names assigned by the programmer
  IDENTIFER,
  
  // Keywords, reserved words in the language, if, while, return
  IF,
  ELSE,
  RETURN,

  // Separators, punctuation characters and paried delimiters,
  LEFT_PARENTHESIS,
  RIGHT_PARENTHESIS,
  LEFT_CURLY_BRACE,
  RIGHT_CURLY_BRACE,
  LEFT_BRACKET,
  RIGHT_BRACKET,
  COMMA,
  NEWLINE,

  // Data type, sint32, uint32, bool, string, char 
  INT,
  SIGNED_INT_32,
  UNSIGNED_INT_32,
  BOOl,
  STRING,
  CHAR,

  // Operators, symbols that operate on arguments and produce results, +, <, =
  PLUS,
  EQUAL,
  
  // Literals,  numbers, logical, textual and refernce literals
  INTEGER_LITERAL,
  STRING_LITERAL,

  COMMENT,
  WHITESPACE,
  END_OF_FILE,
};

// Mapping of reserved types  
const std::unordered_map<std::string, TokenType> reserverdTypeToToken = {
  {"if", IF},
  {"else", ELSE},
  {"return", RETURN},

  {"(", LEFT_PARENTHESIS},
  {")", RIGHT_PARENTHESIS},
  {"{", LEFT_CURLY_BRACE},
  {"}", RIGHT_CURLY_BRACE},
  {"[", LEFT_BRACKET},
  {"]", RIGHT_BRACKET},
  {",", COMMA},

  {"int", INT},
  {"sint32", SIGNED_INT_32},
  {"uint32", UNSIGNED_INT_32},
  {"bool", BOOl},
  {"string", STRING},
  {"char", CHAR},

  {"+", PLUS},
};

// Separator and operators char map..
const std::unordered_map<char, TokenType> separatorOperatorToToken = {
  {'(', LEFT_PARENTHESIS},
  {')', RIGHT_PARENTHESIS},
  {'{', LEFT_CURLY_BRACE},
  {'}', RIGHT_CURLY_BRACE},
  {'[', LEFT_BRACKET},
  {']', RIGHT_BRACKET},
  {',', COMMA},

  {'+', PLUS},
  {'=', EQUAL},
};


} // namespace Lexer