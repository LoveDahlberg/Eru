#include "AST/VariableDeclaration.h"
#include "Support/Result.h"
#include <Parser/Parser.h>

namespace Parser {

Error Parser::ParseVariableDeclarationOrFunction() {
  auto variable = ParseVariable();
  RET_ON_FAILURE_CODE(
      variable, "ParseVariableDeclarationOrFunction: Failed to parse variable",
      lexer);

  if (lexer.getCurrentToken() == TokenType::LEFT_PARENTHESIS) {
    return ParseFunction(*variable);
  }

  std::optional<AST::Expression::ConstantOperand> constOperand;

  // Check if the global value is assigned.
  if (lexer.getCurrentToken() == TokenType::EQUAL) {

    // Pop the equal.
    lexer.generateNextToken();

    auto operand = ParseConstantOperand();

    RET_ON_FAILURE(
        operand,
        "ParseVariableDeclarationOrFunction: Faled to parse Constant Operand");

    constOperand = *operand;
  }

  RET_ON_WRONG_TOKEN(TokenType::NEWLINE,
                     "ParseVariableDeclarationOrFunction: Expected newline "
                     "after global variable declaration.");

  return analyzer.variable().ActOnGlobalDeclaration(*variable, constOperand);
}

Error Parser::ParseTopLevelItems() {
  int loopCounter = 0;
  do {
    // TODO should just be able to skip all newlines here.
    auto tokenCategory = tokenTypeToCategory.at(lexer.generateNextToken().type);
    switch (tokenCategory) {
    case TokenCategory::SEPARATOR: {

      if (lexer.getCurrentToken() != TokenType::LEFT_BRACKET) {
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

  return ERU_SUCCESS;
}

Error Parser::ParseFunctionBodies() {
  for (auto functionBodyToParse : functionBodiesToParse) {
    lexer.restartFromIndex(functionBodyToParse.startIndex);

    auto functionBody = ParseFunctionBody(*functionBodyToParse.function);

    RET_ON_FAILURE_CODE(functionBody,
                        "ParseFunctionBodies: Failed to parse functionBody",
                        lexer);

    RET_ON_FAILURE(analyzer.function().ActOnBody(*functionBody),
                   "ParseFunctionBodies: Failed ActOnBody.");
  }
  return ERU_SUCCESS;
}

} // namespace Parser