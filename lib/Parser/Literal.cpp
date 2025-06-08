
#include <Parser/Parser.h>

namespace Parser {

std::optional<std::string> Parser::ParseLiteral() {
  if (lexer.getCurrentToken().type != TokenType::INTEGER_LITERAL &&
      lexer.getCurrentToken().type != TokenType::STRING_LITERAL) {
    // err
    return std::nullopt;
  }

  // TODO might be issues to treat ints as strings here.
  auto literal = lexer.getCurrentToken().value;

  // Get next, current type saved.
  lexer.generateNextToken();
  return literal;
}

} // namespace Parser