

#include <Parser/Directive.h>

namespace Parser::Directive {

bool ParseDirective(ParserItems &items) {
  if (items.lexer.getCurrentToken().type != TokenType::LEFT_BRACKET) {
    // err
    return false;
  }

  // Eat [
  items.lexer.generateNextToken();

  if (items.lexer.getCurrentToken().type != TokenType::RIGHT_BRACKET) {
    // err
    return false;
  }

  // Eat ]
  items.lexer.generateNextToken();

  return true;
}

} // namespace Parser::Directive