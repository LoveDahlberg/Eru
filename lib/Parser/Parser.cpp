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

  // Parse all top level declarations and definitons.
  // Should this also resolve all directives?
  RET_ON_FAILURE(ParseTopLevelItems(), "Parse: ParseTopLevelItems failed.");
  
  // For each function definition, call ParseBlock.
  RET_ON_FAILURE(ParseFunctionBodies(), "Parse: ParseFunctionBodies failed.");

  return true;
}

} // namespace Parser
