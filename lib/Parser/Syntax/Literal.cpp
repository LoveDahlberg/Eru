
#include <Parser/Syntax/Literal.h>

namespace Parser::Syntax::Literal {

std::optional<std::string> ParseLiteral(syntaxItems &items) {
  if (items.lexer.getCurrentToken().type != TokenType::INTEGER_LITERAL &&
      items.lexer.getCurrentToken().type != TokenType::STRING_LITERAL) {
    // err
    return std::nullopt;
  }

  // TODO might be issues to treat ints as strings here.
  auto literal = items.lexer.getCurrentToken().value;

  // Get next, current type saved.
  items.lexer.generateNextToken();
  return literal;
}

} // namespace Parser::Syntax::Literal