#include <Parser/Syntax/Identifier.h>

namespace Parser::Syntax::Identifier {

std::optional<std::string> ParseIdentifier(syntaxItems &items) {
  if (items.lexer.getCurrentToken().type != TokenType::IDENTIFER) {
    // err
    return std::nullopt;
  }

  auto identifier = items.lexer.getCurrentToken().value;

  // Get next, current type saved.
  items.lexer.generateNextToken();
  return identifier;
}

} // namespace Parser::Syntax::Identifier