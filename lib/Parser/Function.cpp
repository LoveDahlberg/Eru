#include "AST/Function.h"
#include "Support/Result.h"
#include <Parser/Parser.h>

namespace Parser {

Result<AST::Function::FunctionCall *>
Parser::ParseFunctionCall(std::string name) {

  // If name is empty, parse the identifier. Otherwise assume that it was parsed
  // before calling this function.
  if (name.empty()) {
    auto identifier = ParseIdentifier();
    RET_ON_FAILURE_CODE(identifier,
                        "ParseFunctionCall: Failed to parse identifier", lexer);
    name = *identifier;
  }

  RET_ON_WRONG_TOKEN(TokenType::LEFT_PARENTHESIS,
                     "ParseFunctionCall: Expected (");

  // eat the (
  lexer.generateNextToken();

  auto parameters = ParseParameters<AST::Expression::Expression *>();
  RET_ON_FAILURE_CODE(parameters,
                      "ParseFunctionCall: Failed to parse parameters", lexer);

  RET_ON_WRONG_TOKEN(TokenType::RIGHT_PARENTHESIS,
                     "ParseFunctionCall: Expected )");

  // eat the )
  lexer.generateNextToken();

  auto call = new AST::Function::FunctionCall(name, *parameters);

  RET_ON_FAILURE_CODE(analyzer.function().ActOnCall(call),
                      "ParseFunctionCall: ", lexer);

  return call;
}

Result<AST::Function::Block *> Parser::ParseBlock() {
  // TODO refactor so that the push and pop of the Scope happens in here instead
  // of before the call, like how its done for the IR.

  skipUntilNotNewline();

  RET_ON_WRONG_TOKEN(TokenType::LEFT_CURLY_BRACE, "ParseBlock: Expected {");

  // eat the {
  lexer.generateNextToken();

  skipUntilNotNewline();

  auto statement = ParseStatement();
  RET_ON_FAILURE_CODE(statement, "ParseBlock: Failed to parse statement",
                      lexer);

  auto block = new AST::Function::Block(*statement);

  if (lexer.getCurrentToken() == TokenType::RETURN) {
    // eat the return
    lexer.generateNextToken();

    auto expression = ParseExpression();
    RET_ON_FAILURE_CODE(expression, "ParseBlock: Failed to parse expression",
                        lexer);

    block->addReturn(*expression);
  }

  // TODO Error if return is expected but not present..

  skipUntilNotNewline();

  RET_ON_WRONG_TOKEN(TokenType::RIGHT_CURLY_BRACE, "ParseBlock: Expected }");

  // eat the }
  lexer.generateNextToken();

  return block;
}

Result<AST::Function::FunctionBody *>
Parser::ParseFunctionBody(AST::Function::Parameters parameters) {

  analyzer.PushScope();

  // Declare the function parameters as local variables in the current scope.
  RET_ON_FAILURE(analyzer.function().ActOnParameters(parameters),
                 "ParseFunctionBody: failed to act on parameters.");

  auto block = ParseBlock();
  analyzer.PopScope();

  RET_ON_FAILURE_CODE(block, "ParseFunctionBody: Failed to parse block", lexer);

  return new AST::Function::FunctionBody(*block);
}

Error Parser::SkipFunctionBody() {
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

    RET_ON_TRUE_CODE(loopCounter++ > loopLimit,
                     "SkipFunctionBody: Loop limit reached", lexer);

    lexer.generateNextToken();
  };

  return SUCCESS;
}

Error Parser::ParseFunction(AST::VariableDeclaration::Variable *variable) {
  // eat the (
  lexer.generateNextToken();

  auto paramaters = ParseParameters<AST::VariableDeclaration::Variable *>();

  RET_ON_FAILURE_CODE(paramaters, "ParseFunction: Failed to parse parameters",
                      lexer);

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
    RET_ON_FAILURE_CODE(SkipFunctionBody(),
                        "ParseFunction: Failed SkipFunctionBody.", lexer);

    return analyzer.function().ActOnDefinition(function);
  }
  return analyzer.function().ActOnDeclaration(function);
}

} // namespace Parser