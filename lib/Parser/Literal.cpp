
#include <Parser/Parser.h>

namespace Parser {

Result<std::string> Parser::ParseLiteral() {
  if (lexer.getCurrentToken().type != TokenType::INTEGER_LITERAL &&
      lexer.getCurrentToken().type != TokenType::STRING_LITERAL) {
    return FAILURE_CODE("ParseLiteral: expecetd integer or string literal", lexer);
  }

  // TODO might be issues to treat ints as strings here.
  auto literal = lexer.getCurrentToken().value;

  // Get next, current type saved.
  lexer.generateNextToken();
  return literal;
}

} // namespace Parser