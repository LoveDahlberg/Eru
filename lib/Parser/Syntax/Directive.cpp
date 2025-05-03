

#include <Parser/Syntax/Directive.h>

namespace Parser::Syntax::Directive {

bool ParseDirective(syntaxItems &items) {
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

} // namespace Parser::Syntax::Directive