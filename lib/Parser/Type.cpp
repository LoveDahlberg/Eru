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

  if(lexer.getCurrentToken().type == TokenType::AMPERSAND)
  {
    type.isPointer = true;
    lexer.generateNextToken();
  }

  return type;
}

} // namespace Parser