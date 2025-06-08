#include <Parser/Parser.h>

namespace Parser {

std::optional<std::string> Parser::ParseIdentifier() {
  if (lexer.getCurrentToken().type != TokenType::IDENTIFER) {
    // err
    return std::nullopt;
  }

  auto identifier = lexer.getCurrentToken().value;

  // Get next, current type saved.
  lexer.generateNextToken();
  return identifier;
}

} // namespace Parser::Identifier