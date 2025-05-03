
#include <Parser/Syntax/Directive.h>
#include <Parser/Syntax/Function.h>
#include <Parser/Syntax/Statement.h>
#include <Parser/Syntax/VariableDeclaration.h>
#include <Parser/Syntax/Identifier.h>
#include <Parser/Syntax/Expression.h>

namespace Parser::Syntax::Function {

std::optional<FunctionCall *> ParseFunctionCall(syntaxItems &items,
                                                          std::string name) {

  // If name is empty, parse the identifier. Otherwise assume that it was parsed
  // before calling this function.
  if (name.empty()) {
    auto identifier = Identifier::ParseIdentifier(items);
    if (!identifier) {
      // err
      return std::nullopt;
    }
    name = *identifier;
  }

  if (items.lexer.getCurrentToken().type != TokenType::LEFT_PARENTHESIS) {
    // err
    return std::nullopt;
  }

  // eat the (
  items.lexer.generateNextToken();

  auto parameters = ParseParameters<ExpressionUnit *>(
      items, &Expression::ParseExpressionUnit);
  if (!parameters) {
    // err
    return std::nullopt;
  }

  if (items.lexer.getCurrentToken().type != TokenType::RIGHT_PARENTHESIS) {
    // err
    return std::nullopt;
  }

  // eat the )
  items.lexer.generateNextToken();

  // TODO newline here?

  return new FunctionCall(name, *parameters);
}

std::optional<FunctionBody *> ParseFunctionBody(syntaxItems &items) {
  auto directive = Directive::ParseDirective(items);

  skipUntilNotNewline(items);

  if (items.lexer.getCurrentToken().type != TokenType::LEFT_CURLY_BRACE) {
    // err
    return std::nullopt;
  }

  // eat the {
  items.lexer.generateNextToken();

  auto statement = Statement::ParseStatement(items);
  if (!statement) {
    // err
    return std::nullopt;
  }

  skipUntilNotNewline(items);

  if (items.lexer.getCurrentToken().type != TokenType::RIGHT_CURLY_BRACE) {
    // err
    return std::nullopt;
  }

  // eat the }
  items.lexer.generateNextToken();

  return new FunctionBody(*statement);
}

bool ParseFunction(syntaxItems &items, variableDeclarationAST *declaration) {
  // eat the (
  items.lexer.generateNextToken();

  auto paramaters = ParseParameters<variableDeclarationAST *>(
      items, &VariableDeclaration::ParseVariableDeclaration);
  if (!paramaters) {
    // err
    return false;
  }

  if (items.lexer.getCurrentToken().type != TokenType::RIGHT_PARENTHESIS) {
    // err
    return false;
  }

  // Lookahead and get the next non newline token.
  auto lookaheadToken = items.lexer.lookaheadTokenNotNewline();

  // eat the ), needed before leaving this function.
  items.lexer.generateNextToken();

  auto function =
      new functionAST(declaration->type, declaration->name, *paramaters);

  if (lookaheadToken.type == TokenType::LEFT_BRACKET) {
    auto functionBody = ParseFunctionBody(items);
    if (!functionBody) {
      // err
      return false;
    }
    function->addFunctionBody(*functionBody);
  }

  items.compilationUnit.AddCompilationUnitItems(function);
  return true;
}

} // namespace Parser::Syntax::Function