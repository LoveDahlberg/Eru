#include <Parser/Parser.h>

namespace Parser {

Error Parser::ParseVariableDeclarationOrFunction() {
  auto variable = ParseVariable();
  RET_ON_FAILURE_CODE(
      variable, "ParseVariableDeclarationOrFunction: Failed to parse variable",
      lexer);

  if (lexer.getCurrentToken() == TokenType::LEFT_PARENTHESIS) {
    return ParseFunction(*variable);
  } else {

    // TODO should we parse expressions here? If so, only constants should be
    // able to be used.

    RET_ON_WRONG_TOKEN(TokenType::NEWLINE,
                       "ParseVariableDeclarationOrFunction: Expected newline "
                       "after variable declaration");
  }

  return analyzer.variable().ActOnGlobalDeclaration(*variable);
}

Error Parser::ParseTopLevelItems() {
  int loopCounter = 0;
  do {
    // TODO should just be able to skip all newlines here.
    auto tokenCategory = tokenTypeToCategory.at(lexer.generateNextToken().type);
    switch (tokenCategory) {
    case TokenCategory::SEPARATOR: {

      if (lexer.getCurrentToken().type != TokenType::LEFT_BRACKET) {
        continue;
      }

      RET_ON_FAILURE_CODE(ParseDirective(),
                          "ParseCompilationUnit: Call to ParseDirective failed",
                          lexer);
      continue;
    }
    case TokenCategory::DATA_TYPE: {
      RET_ON_FAILURE_CODE(ParseVariableDeclarationOrFunction(),
                          "ParseCompilationUnit: Call to "
                          "ParseVariableDeclarationOrFunction failed",
                          lexer);
      continue;
    }
    default:
      if (lexer.getCurrentToken() == TokenType::END_OF_FILE) {
        break;
      }
      return FAILURE_CODE("ParseCompilationUnit: Unexpected token.", lexer);
    }
    // Break the main loop
    break;
  } while (loopCounter++ < loopLimit);

  return SUCCESS;
}

Error Parser::ParseFunctionBodies() {
  for (auto functionBodyToParse : functionBodiesToParse) {
    lexer.restartFromIndex(functionBodyToParse.startIndex);

    auto functionBody =
        ParseFunctionBody(functionBodyToParse.function->parameters);

    RET_ON_FAILURE_CODE(functionBody,
                        "ParseFunctionBodies: Failed to parse functionBody",
                        lexer);

    functionBodyToParse.function->addFunctionBody(*functionBody);
  }
  return SUCCESS;
}

} // namespace Parser