
// include
#include <Parser/CompilationUnit.h>
#include <Parser/Directive.h>
#include <Parser/Function.h>
#include <Parser/Identifier.h>
#include <Parser/Type.h>
#include <Parser/VariableDeclaration.h>

namespace Parser {

bool ParseVariableDeclarationOrFunction(Parser &ctx) {
  auto variable =
      VariableDeclaration::ParseVariable(ctx);
  if (!variable) {
    // err
    return false;
  }

  if (ctx.lexer.getCurrentToken().type == TokenType::LEFT_PARENTHESIS) {
    return Function::ParseFunction(ctx, *variable);
  }

  ctx.compilationUnit.AddCompilationUnitItems(new variableDeclarationAST(*variable));
  return true;
}

// TODO improve error handling
bool ParseCompilationUnit(Parser &ctx) {
  int loopCounter = 0;
  do {
    // TODO should just be able to skip all newlines here.
    auto tokenCategory =
        tokenTypeToCategory.at(ctx.lexer.generateNextToken().type);
    switch (tokenCategory) {
    case TokenCategory::SEPARATOR:
      if (ctx.lexer.getCurrentToken().type != TokenType::LEFT_BRACKET) {
        continue;
      }

      if (Directive::ParseDirective(ctx)) {
        continue;
      }
      return false;
    case TokenCategory::DATA_TYPE:
      if (ParseVariableDeclarationOrFunction(ctx)) {
        continue;
      }
      return false;
    default:
      if (ctx.lexer.getCurrentToken().type == TokenType::END_OF_FILE) {
        break;
      }
      // err
      // printParsing(CompilationUnitParsingName, tokenCategory,
      //              items.lexer.getCurrentToken());
      return false;
    }
    // Break the main loop
    break;
  } while (loopCounter++ < loopLimit);

  return true;
}

} // namespace Parser