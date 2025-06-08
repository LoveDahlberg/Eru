#include <Parser/Parser.h>

namespace Parser {

bool Parser::ParseDirective() {
  if (lexer.getCurrentToken().type != TokenType::LEFT_BRACKET) {
    // err
    return false;
  }

  // Eat [
  lexer.generateNextToken();

  if (lexer.getCurrentToken().type != TokenType::RIGHT_BRACKET) {
    // err
    return false;
  }

  // Eat ]
  lexer.generateNextToken();

  return true;
}

} // namespace Parser::Directive