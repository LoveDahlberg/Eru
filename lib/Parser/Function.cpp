
#include <Parser/Directive.h>
#include <Parser/Expression.h>
#include <Parser/Function.h>
#include <Parser/Identifier.h>
#include <Parser/Statement.h>
#include <Parser/VariableDeclaration.h>

namespace Parser::Function {

std::optional<FunctionCall *> ParseFunctionCall(ParserItems &items,
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

  auto parameters =
      ParseParameters<expressionAST *>(items, &Expression::ParseExpression);
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

std::optional<Block *> ParseBlock(ParserItems &items) {
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

  auto block = new Block(*statement);

  if (items.lexer.getCurrentToken().type == TokenType::RETURN) {
    // eat the return
    items.lexer.generateNextToken();

    auto expression = Expression::ParseExpression(items);
    if (!expression) {
      // err
      return std::nullopt;
    }
    block->addReturn(*expression);
  }

  skipUntilNotNewline(items);

  if (items.lexer.getCurrentToken().type != TokenType::RIGHT_CURLY_BRACE) {
    // err
    return std::nullopt;
  }

  // eat the }
  items.lexer.generateNextToken();

  return block;
}

std::optional<FunctionBody *> ParseFunctionBody(ParserItems &items) {
  auto directive = Directive::ParseDirective(items);

  auto block = ParseBlock(items);
  if (!block) {
    // err
    return std::nullopt;
  }

  return new FunctionBody(*block);
}

bool ParseFunction(ParserItems &items, Variable *variable) {
  // eat the (
  items.lexer.generateNextToken();

  auto paramaters =
      ParseParameters<Variable *>(items, &VariableDeclaration::ParseVariable);
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

  auto function = new functionAST(variable->type, variable->name, *paramaters);

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

} // namespace Parser::Function