#include <Parser/Parser.h>

namespace Parser {

Result<bool> Parser::ParseDirective() {
  RET_ON_WRONG_TOKEN(TokenType::LEFT_BRACKET, "ParseDirective: expected [");
  
  // Eat [
  lexer.generateNextToken();

  RET_ON_WRONG_TOKEN(TokenType::RIGHT_BRACKET, "ParseDirective: expected ]");

  // Eat ]
  lexer.generateNextToken();

  return true;
}

} // namespace Parser::Directive