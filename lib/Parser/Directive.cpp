#include <Parser/Parser.h>

namespace Parser {

Result<bool> Parser::ParseDirective() {

  if (lexer.getCurrentToken().type != TokenType::LEFT_BRACKET) {
    return true;
  }

  // Eat [
  lexer.generateNextToken();

  RET_ON_WRONG_TOKEN(TokenType::RIGHT_BRACKET, "ParseDirective: expected ]");

  // Eat ]
  lexer.generateNextToken();

  return true;
}

} // namespace Parser