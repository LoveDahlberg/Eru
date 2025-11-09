#include "Support/Result.h"
#include "Support/Scope.h"
#include <AST/Function.h>
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

Result<AST::Function::Block *>
Parser::ParseBlock(Support::Scope::scopeKind kind) {

  analyzer.PushScope(kind);

  RET_ON_FAILURE(analyzer.function().ActOnParameters(),
                 "ParseBlock: failed to act on parameters.");

  skipUntilNotNewline();

  RET_ON_WRONG_TOKEN(TokenType::LEFT_CURLY_BRACE, "ParseBlock: Expected {");

  // eat the {
  lexer.generateNextToken();

  skipUntilNotNewline();

  auto statement = ParseStatement();
  RET_ON_FAILURE_CODE(statement, "ParseBlock: Failed to parse statement",
                      lexer);

  auto block = new AST::Function::Block(*statement, kind);

  auto returnType = AST::Types::NONE;
  if (lexer.getCurrentToken() == TokenType::RETURN) {
    // eat the return
    lexer.generateNextToken();

    auto expression = ParseExpression();
    RET_ON_FAILURE_CODE(expression, "ParseBlock: Failed to parse expression",
                        lexer);

    returnType = (*expression)->evaluatedType;

    block->addReturn(*expression);
  }

  RET_ON_FAILURE_CODE(analyzer.function().ActOnReturnValue(returnType),
                      "ParseBlock: Failed to act on return value", lexer);

  skipUntilNotNewline();

  RET_ON_WRONG_TOKEN(TokenType::RIGHT_CURLY_BRACE, "ParseBlock: Expected }");

  // eat the }
  lexer.generateNextToken();

  analyzer.PopScope();

  return block;
}

Result<AST::Function::FunctionBody *>
Parser::ParseFunctionBody(AST::Function::FunctionDeclaration declaration) {

  // Set new contextData for the upcoming function scope.
  analyzer.PrepareFunctionScope(
      new Analyzer::AnalyzerScopeContextData{.declaration = declaration});

  auto block = ParseBlock(Support::Scope::scopeKind::FUNCTION);
  RET_ON_FAILURE_CODE(block, "ParseFunctionBody: Failed to parse block", lexer);

  return new AST::Function::FunctionBody(declaration.name, *block);
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

  // TODO add parsing of the main function Valinor here.

  // eat the (
  lexer.generateNextToken();

  auto paramaters = ParseParameters<AST::VariableDeclaration::Variable *>();

  RET_ON_FAILURE_CODE(paramaters, "ParseFunction: Failed to parse parameters",
                      lexer);

  RET_ON_WRONG_TOKEN(TokenType::RIGHT_PARENTHESIS, "ParseFunction: Expected )");

  // eat the ), needed before leaving this function.
  lexer.generateNextToken();

  auto directive = ParseDirective();

  auto function = new AST::Function::FunctionDeclaration(
      variable->type, variable->name, *paramaters);

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