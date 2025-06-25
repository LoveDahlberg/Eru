#include <Parser/Parser.h>

namespace Parser {

Result<bool> Parser::ParseVariableDeclarationOrFunction() {
  auto variable = ParseVariable();
  RET_ON_FAILURE(
      variable, "ParseVariableDeclarationOrFunction: Failed to parse variable");

  if (lexer.getCurrentToken().type == TokenType::LEFT_PARENTHESIS) {
    return ParseFunction(*variable);
  }

  analyzer.ActOnVariableDeclaration(*variable);

  return true;
}

Result<bool> Parser::ParseCompilationUnit() {
  int loopCounter = 0;
  do {
    // TODO should just be able to skip all newlines here.
    auto tokenCategory = tokenTypeToCategory.at(lexer.generateNextToken().type);
    switch (tokenCategory) {
    case TokenCategory::SEPARATOR: {

      if (lexer.getCurrentToken().type != TokenType::LEFT_BRACKET) {
        continue;
      }

      RET_ON_FAILURE(ParseDirective(),
                     "ParseCompilationUnit: Call to ParseDirective failed");
      continue;
    }
    case TokenCategory::DATA_TYPE: {
      RET_ON_FAILURE(ParseVariableDeclarationOrFunction(),
                     "ParseCompilationUnit: Call to "
                     "ParseVariableDeclarationOrFunction failed");
      continue;
    }
    default:
      if (lexer.getCurrentToken().type == TokenType::END_OF_FILE) {
        break;
      }
      return {"ParseCompilationUnit: Unexpected token."};
    }
    // Break the main loop
    break;
  } while (loopCounter++ < loopLimit);

  return true;
}

} // namespace Parser