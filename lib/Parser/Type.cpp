#include <Parser/Parser.h>

using namespace AST::Types;

namespace Parser {


Result<Type> Parser::ParseType() {
  Type type;

  // TODO utilize Analyzer when types can be custom.
  switch (lexer.getCurrentToken().type) {
  case TokenType::INT:
    type.dataType = DataType::INT;
    break;
  case TokenType::SIGNED_INT_32:
    type.dataType = DataType::SINT32;
    break;
  case TokenType::UNSIGNED_INT_32:
    type.dataType = DataType::UINT32;
    break;
  case TokenType::BOOl:
    type.dataType = DataType::BOOl;
    break;
  case TokenType::CHAR:
    type.dataType = DataType::CHAR;
    break;
  case TokenType::STRING:
    // TODO implement string handling
    type.dataType = DataType::STRING;
    break;
  default:
    return FAILURE_CODE(Formatter("ParseType: invalid type '",
                                  lexer.getCurrentToken().value, "'"),
                        lexer);
  }

  lexer.generateNextToken();

  if (lexer.getCurrentToken() == TokenType::AMPERSAND) {
    // Type is pointer if it contains at least one &.
    type.isPointer = true;

    // Can contain any number of &, go through them all.
    type.pointerDepth = 1;
    do {
      lexer.generateNextToken();
      if (lexer.getCurrentToken() != TokenType::AMPERSAND) {
        break;
      }
    } while (++type.pointerDepth < loopLimit);
  }

  return type;
}

} // namespace Parser