#include <Parser/Parser.h>

namespace Parser {

std::optional<AST::Function::FunctionCall *>
Parser::ParseFunctionCall(std::string name) {

  // If name is empty, parse the identifier. Otherwise assume that it was parsed
  // before calling this function.
  if (name.empty()) {
    auto identifier = ParseIdentifier();
    if (identifier.hasFailed) {
      // err
      return std::nullopt;
    }
    name = *identifier;
  }

  if (lexer.getCurrentToken().type != TokenType::LEFT_PARENTHESIS) {
    // err
    return std::nullopt;
  }

  // eat the (
  lexer.generateNextToken();

  auto parameters = ParseParameters<AST::Expression::Expression *>();
  if (!parameters) {
    // err
    return std::nullopt;
  }

  if (lexer.getCurrentToken().type != TokenType::RIGHT_PARENTHESIS) {
    // err
    return std::nullopt;
  }

  // eat the )
  lexer.generateNextToken();

  // TODO newline here?

  return new AST::Function::FunctionCall(name, *parameters);
}

std::optional<AST::Function::Block *> Parser::ParseBlock() {
  skipUntilNotNewline();

  if (lexer.getCurrentToken().type != TokenType::LEFT_CURLY_BRACE) {
    // err
    return std::nullopt;
  }

  // eat the {
  lexer.generateNextToken();

  auto statement = ParseStatement();
  if (!statement) {
    // err
    return std::nullopt;
  }

  auto block = new AST::Function::Block(*statement);

  if (lexer.getCurrentToken().type == TokenType::RETURN) {
    // eat the return
    lexer.generateNextToken();

    auto expression = ParseExpression();
    if (!expression) {
      // err
      return std::nullopt;
    }
    block->addReturn(*expression);
  }

  skipUntilNotNewline();

  if (lexer.getCurrentToken().type != TokenType::RIGHT_CURLY_BRACE) {
    // err
    return std::nullopt;
  }

  // eat the }
  lexer.generateNextToken();

  return block;
}

std::optional<AST::Function::FunctionBody *> Parser::ParseFunctionBody() {
  auto directive = ParseDirective();

  auto block = ParseBlock();
  if (!block) {
    // err
    return std::nullopt;
  }

  return new AST::Function::FunctionBody(*block);
}

bool Parser::ParseFunction(AST::VariableDeclaration::Variable *variable) {
  // eat the (
  lexer.generateNextToken();

  auto paramaters = ParseParameters<AST::VariableDeclaration::Variable *>();
  if (!paramaters) {
    // err
    return false;
  }

  if (lexer.getCurrentToken().type != TokenType::RIGHT_PARENTHESIS) {
    // err
    return false;
  }

  // Lookahead and get the next non newline token.
  auto lookaheadToken = lexer.lookaheadTokenNotNewline();

  // eat the ), needed before leaving this function.
  lexer.generateNextToken();

  auto function =
      new AST::Function::Function(variable->type, variable->name, *paramaters);

  if (lookaheadToken.type == TokenType::LEFT_BRACKET) {

    analyzer.PushScope();
    auto functionBody = ParseFunctionBody();
    analyzer.PopScope();

    if (!functionBody) {
      // err
      return false;
    }
    function->addFunctionBody(*functionBody);

    analyzer.ActOnFunctionImplementation(function);
  } else {
    analyzer.ActOnFunctionDeclaration(function);
  }

  // astContext.compilationUnit->AddCompilationUnitItems(function);
  return true;
}

} // namespace Parser