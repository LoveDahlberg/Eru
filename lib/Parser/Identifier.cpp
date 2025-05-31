#include <Parser/Identifier.h>

namespace Parser::Identifier {

std::optional<std::string> ParseIdentifier(ParserItems &items) {
  if (items.lexer.getCurrentToken().type != TokenType::IDENTIFER) {
    // err
    return std::nullopt;
  }

  auto identifier = items.lexer.getCurrentToken().value;

  // Get next, current type saved.
  items.lexer.generateNextToken();
  return identifier;
}

} // namespace Parser::Identifier