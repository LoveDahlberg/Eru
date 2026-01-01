#include <Parser/Parser.h>

namespace Parser {

Result<std::string> Parser::ParseIdentifier() {

  RET_ON_WRONG_TOKEN(
      TokenType::IDENTIFER,
      "ParseIdentifier: wrong token. Expected Identifier got");

  auto identifier = lexer.getCurrentToken().value;

  // Get next, current type saved.
  lexer.generateNextToken();
  return identifier;
}

} // namespace Parser