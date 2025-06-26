#include <Parser/Parser.h>

namespace Parser {

Result<AST::Function::FunctionCall *>
Parser::ParseFunctionCall(std::string name) {

  // If name is empty, parse the identifier. Otherwise assume that it was parsed
  // before calling this function.
  if (name.empty()) {
    auto identifier = ParseIdentifier();
    RET_ON_FAILURE(identifier, "ParseFunctionCall: Failed to parse identifier");
    name = *identifier;
  }

  RET_ON_WRONG_TOKEN(TokenType::LEFT_PARENTHESIS,
                     "ParseFunctionCall: Expected (");

  // eat the (
  lexer.generateNextToken();

  auto parameters = ParseParameters<AST::Expression::Expression *>();
  RET_ON_FAILURE(parameters, "ParseFunctionCall: Failed to parse parameters");

  RET_ON_WRONG_TOKEN(TokenType::RIGHT_PARENTHESIS,
                     "ParseFunctionCall: Expected )");

  // eat the )
  lexer.generateNextToken();

  // TODO newline here?

  return new AST::Function::FunctionCall(name, *parameters);
}

Result<AST::Function::Block *> Parser::ParseBlock() {
  skipUntilNotNewline();

  RET_ON_WRONG_TOKEN(TokenType::LEFT_CURLY_BRACE, "ParseBlock: Expected {");

  // eat the {
  lexer.generateNextToken();

  auto statement = ParseStatement();
  RET_ON_FAILURE(statement, "ParseBlock: Failed to parse statement");

  auto block = new AST::Function::Block(*statement);

  if (lexer.getCurrentToken().type == TokenType::RETURN) {
    // eat the return
    lexer.generateNextToken();

    auto expression = ParseExpression();
    RET_ON_FAILURE(expression, "ParseBlock: Failed to parse expression");

    block->addReturn(*expression);
  }

  skipUntilNotNewline();

  RET_ON_WRONG_TOKEN(TokenType::RIGHT_CURLY_BRACE, "ParseBlock: Expected }");

  // eat the }
  lexer.generateNextToken();

  return block;
}

Result<AST::Function::FunctionBody *> Parser::ParseFunctionBody() {
  auto directive = ParseDirective();

  auto block = ParseBlock();

  RET_ON_FAILURE(block, "ParseFunctionBody: Failed to parse block");

  return new AST::Function::FunctionBody(*block);
}

Result<bool>
Parser::ParseFunction(AST::VariableDeclaration::Variable *variable) {
  // eat the (
  lexer.generateNextToken();

  auto paramaters = ParseParameters<AST::VariableDeclaration::Variable *>();

  RET_ON_FAILURE(paramaters, "ParseFunction: Failed to parse parameters");

  RET_ON_WRONG_TOKEN(TokenType::RIGHT_PARENTHESIS, "ParseFunction: Expected )");

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

    RET_ON_FAILURE(functionBody, "ParseFunction: Failed to parse functionBody");

    function->addFunctionBody(*functionBody);

    return analyzer.ActOnFunctionDefinition(function);
  }
  return analyzer.ActOnFunctionDeclaration(function);
}

} // namespace Parser