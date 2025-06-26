#include <Parser/Parser.h>

namespace Parser {

using Type = AST::Types::Types;

Result<Type> Parser::ParseType() {
  Type type;

  // TODO utilize Analyzer when types can be custom.
  switch (lexer.getCurrentToken().type) {
  case TokenType::INT:
    type = Type::INT;
    break;
  case TokenType::SIGNED_INT_32:
    type = Type::SINT32;
    break;
  case TokenType::UNSIGNED_INT_32:
    type = Type::UINT32;
    break;
  case TokenType::BOOl:
    type = Type::BOOl;
    break;
  case TokenType::CHAR:
    type = Type::CHAR;
    break;
  case TokenType::STRING:
    // TODO implement string handling
    type = Type::STRING;
    break;
  default:
    return {"ParseType: invalid type .."};
  }

  // Get next, current type saved.
  lexer.generateNextToken();
  return type;
}

} // namespace Parser