#include <Parser/Parser.h>

namespace Parser {

bool Parser::ParseVariableDeclarationOrFunction() {
  auto variable = ParseVariable();
  if (!variable) {
    // err
    return false;
  }

  if (lexer.getCurrentToken().type == TokenType::LEFT_PARENTHESIS) {
    return ParseFunction(*variable);
  }

  astContext.compilationUnit->AddCompilationUnitItems(
      new AST::VariableDeclaration::VariableDeclaration(*variable));
  return true;
}

// TODO improve error handling
bool Parser::ParseCompilationUnit() {
  int loopCounter = 0;
  do {
    // TODO should just be able to skip all newlines here.
    auto tokenCategory =
        tokenTypeToCategory.at(lexer.generateNextToken().type);
    switch (tokenCategory) {
    case TokenCategory::SEPARATOR:
      if (lexer.getCurrentToken().type != TokenType::LEFT_BRACKET) {
        continue;
      }

      if (ParseDirective()) {
        continue;
      }
      return false;
    case TokenCategory::DATA_TYPE:
      if (ParseVariableDeclarationOrFunction()) {
        continue;
      }
      return false;
    default:
      if (lexer.getCurrentToken().type == TokenType::END_OF_FILE) {
        break;
      }
      // err
      return false;
    }
    // Break the main loop
    break;
  } while (loopCounter++ < loopLimit);

  return true;
}

} // namespace Parser