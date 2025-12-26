#include <Parser/Parser.h>

namespace Parser {

Error Parser::ParseDirective() {

  if (lexer.getCurrentToken() != TokenType::LEFT_BRACKET) {
    return SUCCESS;
  }

  // Eat [
  lexer.generateNextToken();

  RET_ON_WRONG_TOKEN(TokenType::RIGHT_BRACKET, "ParseDirective: expected ]");

  // Eat ]
  lexer.generateNextToken();

  return SUCCESS;
}

} // namespace Parser