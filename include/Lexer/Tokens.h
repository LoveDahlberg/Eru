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
  ELIF,
  RETURN,
  OR,
  AND,


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
  MINUS,
  EQUAL,
  
  // Literals,  numbers, logical, textual and refernce literals
  INTEGER_LITERAL,
  STRING_LITERAL,

  END_OF_FILE,
};

// Mapping of reserved types
// TODO statically assert that this map is correct, i.e no duplicate values.
const std::unordered_map<std::string, TokenType> reserverdTypeToToken = {
  {"if", IF},
  {"else", ELSE},
  {"elif", ELIF},
  {"return", RETURN},
  {"or", OR},
  {"and", AND},


  {"int", INT},
  {"sint32", SIGNED_INT_32},
  {"uint32", UNSIGNED_INT_32},
  {"bool", BOOl},
  {"string", STRING},
  {"char", CHAR},
};

// Separator and operators char mapping
// TODO statically assert that this map is correct, i.e no duplicate values.
const std::unordered_map<char, TokenType> separatorOperatorToToken = {
  {'(', LEFT_PARENTHESIS},
  {')', RIGHT_PARENTHESIS},
  {'{', LEFT_CURLY_BRACE},
  {'}', RIGHT_CURLY_BRACE},
  {'[', LEFT_BRACKET},
  {']', RIGHT_BRACKET},
  {',', COMMA},

  {'+', PLUS},
  {'-', MINUS},
  {'=', EQUAL},
};


} // namespace Lexer