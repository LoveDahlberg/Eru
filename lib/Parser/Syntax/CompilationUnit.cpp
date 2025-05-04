
// include
#include <Parser/Syntax/CompilationUnit.h>
#include <Parser/Syntax/Directive.h>
#include <Parser/Syntax/Function.h>
#include <Parser/Syntax/Identifier.h>
#include <Parser/Syntax/Type.h>
#include <Parser/Syntax/VariableDeclaration.h>

namespace Parser::Syntax {

bool ParseVariableDeclarationOrFunction(syntaxItems &items) {
  auto variable =
      VariableDeclaration::ParseVariable(items);
  if (!variable) {
    // err
    return false;
  }

  if (items.lexer.getCurrentToken().type == TokenType::LEFT_PARENTHESIS) {
    return Function::ParseFunction(items, *variable);
  }

  items.compilationUnit.AddCompilationUnitItems(new variableDeclarationAST(*variable));
  return true;
}

// TODO improve error handling
std::optional<syntaxItems> ParseCompilationUnit(Lexer &lexer) {
  syntaxItems items(lexer);

  int loopCounter = 0;
  do {
    // TODO should just be able to skip all newlines here.
    auto tokenCategory =
        tokenTypeToCategory.at(items.lexer.generateNextToken().type);
    switch (tokenCategory) {
    case TokenCategory::SEPARATOR:
      if (items.lexer.getCurrentToken().type != TokenType::LEFT_BRACKET) {
        continue;
      }

      if (Directive::ParseDirective(items)) {
        continue;
      }
      return std::nullopt;
    case TokenCategory::DATA_TYPE:
      if (ParseVariableDeclarationOrFunction(items)) {
        continue;
      }
      return std::nullopt;
    default:
      if (items.lexer.getCurrentToken().type == TokenType::END_OF_FILE) {
        break;
      }
      // err
      // printParsing(CompilationUnitParsingName, tokenCategory,
      //              items.lexer.getCurrentToken());
      return std::nullopt;
    }
    // Break the main loop
    break;
  } while (loopCounter++ < loopLimit);

  return items;
}

} // namespace Parser::Syntax