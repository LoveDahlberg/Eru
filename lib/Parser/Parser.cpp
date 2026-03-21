#include <Parser/Parser.h>

namespace Parser {

void Parser::skipUntilNotNewline() {
  if (lexer.getCurrentToken() != TokenType::NEWLINE) {
    return;
  }

  int loopCounter = 0;
  while (lexer.generateNextToken().type == TokenType::NEWLINE) {
    if (lexer.getCurrentToken() == TokenType::END_OF_FILE) {
      return;
    }
    if (loopCounter++ > loopLimit) {
      return;
    }
  }
}

Error Parser::Parse() { 

  // Parse all top level declarations and definitons.
  // Should this also resolve all directives?
  RET_ON_FAILURE_CODE(ParseTopLevelItems(), "Parse: ParseTopLevelItems failed.", lexer);
  
  // For each function definition, call ParseBlock.
  RET_ON_FAILURE_CODE(ParseFunctionBodies(), "Parse: ParseFunctionBodies failed.", lexer);

  return ERU_SUCCESS;
}

} // namespace Parser
