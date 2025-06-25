#include <Parser/Parser.h>

namespace Parser {

void Parser::skipUntilNotNewline() {
  if (lexer.getCurrentToken().type != TokenType::NEWLINE) {
    return;
  }

  int loopCounter = 0;
  while (lexer.generateNextToken().type == TokenType::NEWLINE) {
    if (lexer.getCurrentToken().type == TokenType::END_OF_FILE) {
      return;
    }
    if (loopCounter++ > loopLimit) {
      return;
    }
  }
}

Result<bool> Parser::Parse() { 
  return ParseCompilationUnit();
}

} // namespace Parser
