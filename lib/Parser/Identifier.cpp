#include <Parser/Parser.h>
#include <optional>

namespace Parser {

  Result<std::string> Parser::ParseIdentifier() {
  if (lexer.getCurrentToken().type != TokenType::IDENTIFER) {
    // err
    return {"ParseIdentifier: wrong token. Expected Identifier got ..."};
  }

  auto identifier = lexer.getCurrentToken().value;

  // Get next, current type saved.
  lexer.generateNextToken();
  return identifier;
}

} // namespace Parser::Identifier