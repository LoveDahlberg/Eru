#pragma once

#include <string>
#include <unordered_map>

namespace Lexing {

enum class TokenType {
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
  INT, // TODO, int is currently the same as SIGNED_INT_32. THis is stupid
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
    {"if", TokenType::IF},
    {"else", TokenType::ELSE},
    {"elif", TokenType::ELIF},
    {"return", TokenType::RETURN},
    {"or", TokenType::OR},
    {"and", TokenType::AND},

    {"int", TokenType::INT},
    {"sint32", TokenType::SIGNED_INT_32},
    {"uint32", TokenType::UNSIGNED_INT_32},
    {"bool", TokenType::BOOl},
    {"string", TokenType::STRING},
    {"char", TokenType::CHAR},
};

// Separator and operators char mapping
// TODO statically assert that this map is correct, i.e no duplicate values.
const std::unordered_map<char, TokenType> separatorOperatorToToken = {
    {'(', TokenType::LEFT_PARENTHESIS},
    {')', TokenType::RIGHT_PARENTHESIS},
    {'{', TokenType::LEFT_CURLY_BRACE},
    {'}', TokenType::RIGHT_CURLY_BRACE},
    {'[', TokenType::LEFT_BRACKET},
    {']', TokenType::RIGHT_BRACKET},
    {',', TokenType::COMMA},

    {'+', TokenType::PLUS},
    {'-', TokenType::MINUS},
    {'=', TokenType::EQUAL},
};

enum class TokenCategory {
  NONE,
  IDENTIFER,
  KEYWORD,
  SEPARATOR,
  DATA_TYPE,
  OPERATOR,
  LITERAL,
};

const std::unordered_map<TokenType, TokenCategory> tokenTypeToCategory{
    {TokenType::NONE, TokenCategory::NONE},

    {TokenType::IDENTIFER, TokenCategory::IDENTIFER},

    {TokenType::IF, TokenCategory::KEYWORD},
    {TokenType::ELSE, TokenCategory::KEYWORD},
    {TokenType::ELIF, TokenCategory::KEYWORD},
    {TokenType::RETURN, TokenCategory::KEYWORD},
    {TokenType::OR, TokenCategory::KEYWORD},
    {TokenType::AND, TokenCategory::KEYWORD},

    {TokenType::LEFT_PARENTHESIS, TokenCategory::SEPARATOR},
    {TokenType::RIGHT_PARENTHESIS, TokenCategory::SEPARATOR},
    {TokenType::LEFT_CURLY_BRACE, TokenCategory::SEPARATOR},
    {TokenType::RIGHT_CURLY_BRACE, TokenCategory::SEPARATOR},
    {TokenType::LEFT_BRACKET, TokenCategory::SEPARATOR},
    {TokenType::RIGHT_BRACKET, TokenCategory::SEPARATOR},
    {TokenType::COMMA, TokenCategory::SEPARATOR},
    {TokenType::NEWLINE, TokenCategory::SEPARATOR},

    {TokenType::INT, TokenCategory::DATA_TYPE},
    {TokenType::SIGNED_INT_32, TokenCategory::DATA_TYPE},
    {TokenType::UNSIGNED_INT_32, TokenCategory::DATA_TYPE},
    {TokenType::BOOl, TokenCategory::DATA_TYPE},
    {TokenType::STRING, TokenCategory::DATA_TYPE},
    {TokenType::CHAR, TokenCategory::DATA_TYPE},

    {TokenType::PLUS, TokenCategory::OPERATOR},
    {TokenType::MINUS, TokenCategory::OPERATOR},
    {TokenType::EQUAL, TokenCategory::OPERATOR},

    {TokenType::INTEGER_LITERAL, TokenCategory::LITERAL},
    {TokenType::STRING_LITERAL, TokenCategory::LITERAL},
    {TokenType::END_OF_FILE, TokenCategory::NONE},
};

} // namespace Lexing