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

  auto call = new AST::Function::FunctionCall(name, *parameters);

  RET_ON_FAILURE(analyzer.function().ActOnCall(call), "ParseFunctionCall: ");

  return call;
}

Result<AST::Function::Block *> Parser::ParseBlock() {
  skipUntilNotNewline();

  RET_ON_WRONG_TOKEN(TokenType::LEFT_CURLY_BRACE, "ParseBlock: Expected {");

  // eat the {
  lexer.generateNextToken();

  auto statement = ParseStatement();
  RET_ON_FAILURE(statement, "ParseBlock: Failed to parse statement");

  auto block = new AST::Function::Block(*statement);

  if (lexer.getCurrentToken() == TokenType::RETURN) {
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

  analyzer.PushScope();
  auto block = ParseBlock();
  analyzer.PopScope();

  RET_ON_FAILURE(block, "ParseFunctionBody: Failed to parse block");

  return new AST::Function::FunctionBody(*block);
}

Result<bool> Parser::SkipFunctionBody() {
  if (lexer.getCurrentToken() != TokenType::LEFT_CURLY_BRACE) {
    skipUntilNotNewline();
    RET_ON_WRONG_TOKEN(TokenType::LEFT_CURLY_BRACE,
                       "SkipFunctionBody: Expected {");
  }

  // Eat the {
  lexer.generateNextToken();

  int curlyBraceCount = 1;

  int loopCounter = 0;
  for (;;) {
    if (lexer.getCurrentToken() == TokenType::LEFT_CURLY_BRACE) {
      ++curlyBraceCount;
    }
    if (lexer.getCurrentToken() == TokenType::RIGHT_CURLY_BRACE) {
      --curlyBraceCount;
    }

    if (curlyBraceCount < 1) {
      break;
    }

    RET_ON_TRUE(loopCounter++ > loopLimit,
                "SkipFunctionBody: Loop limit reached");

    lexer.generateNextToken();
  };

  return true;
}

Result<bool>
Parser::ParseFunction(AST::VariableDeclaration::Variable *variable) {
  // eat the (
  lexer.generateNextToken();

  auto paramaters = ParseParameters<AST::VariableDeclaration::Variable *>();

  RET_ON_FAILURE(paramaters, "ParseFunction: Failed to parse parameters");

  RET_ON_WRONG_TOKEN(TokenType::RIGHT_PARENTHESIS, "ParseFunction: Expected )");

  // eat the ), needed before leaving this function.
  lexer.generateNextToken();

  auto directive = ParseDirective();

  auto function =
      new AST::Function::Function(variable->type, variable->name, *paramaters);

  // Lookahead and get the next non newline token.
  auto lookaheadToken = lexer.lookaheadTokenNotNewline();

  // If the current or next non-newline token is a {, then its a function
  // defintion.
  if (lexer.getCurrentToken() == TokenType::LEFT_CURLY_BRACE ||
      lookaheadToken.type == TokenType::LEFT_CURLY_BRACE) {

    // Don't parse the function body yet. This is done in another pass over the
    // code.
    functionBodiesToParse.push_back({lexer.getCurrentIndex(), function});

    // Instead skip over the entire function body.
    RET_ON_FAILURE(SkipFunctionBody(),
                   "ParseFunction: Failed SkipFunctionBody.");

    return analyzer.function().ActOnDefinition(function);
  }
  return analyzer.function().ActOnDeclaration(function);
}

} // namespace Parser