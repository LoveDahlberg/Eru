#include <Parser/Parser.h>

namespace Parser {

Error Parser::ParseDirective() {

  if (lexer.getCurrentToken() != TokenType::LEFT_BRACKET) {
    return SUCCESSFUL;
  }

  // Eat [
  lexer.generateNextToken();

  RET_ON_WRONG_TOKEN(TokenType::RIGHT_BRACKET, "ParseDirective: expected ]");

  // Eat ]
  lexer.generateNextToken();

  return SUCCESSFUL;
}

} // namespace Parser