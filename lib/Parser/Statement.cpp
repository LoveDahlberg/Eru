#include "Lexer/Tokens.h"
#include "Support/Result.h"
#include <Parser/Parser.h>

namespace Parser {

Result<AST::Statement::StatementVariant>
Parser::ParseFunctionCallOrAssignment(int indirection) {

  // Both start with an identifier.
  auto identifier = ParseIdentifier();
  RET_ON_FAILURE_CODE(identifier,
                      "ParseStatement: identifier: failed identifier", lexer);

  // TODO move this switch case to getter function.
  switch (lexer.getCurrentToken().type) {

  // Assignment
  case TokenType::EQUAL: {
    auto assignment = ParseAssignment(new AST::VariableDeclaration::Variable(
        AST::Types::DataType::NONE, *identifier));
    RET_ON_FAILURE_CODE(assignment,
                        "ParseStatement: identifier: equal: failed assignment",
                        lexer);

    // Run the assignment through the analyzer.
    RET_ON_FAILURE_CODE(
        analyzer.variable().ActOnAssignment(*assignment, indirection),
        "ParseStatement: identifier: equal: failed to act on assignment.",
        lexer);

    return (AST::Statement::StatementVariant)*assignment;
  }

  // Function call.
  case TokenType::LEFT_PARENTHESIS: {
    auto assignment = ParseFunctionCall(*identifier);
    RET_ON_FAILURE_CODE(
        assignment, "ParseStatement: identifier: left paran: failed assignment",
        lexer);

    return (AST::Statement::StatementVariant)*assignment;
  }
  default: {
    return FAILURE_CODE("ParseStatement: identifier: unexpected token", lexer);
  }
  }
  return SUCCESSFUL;
}

Result<std::vector<AST::Statement::StatementVariant>>
Parser::ParseVaribleAndMaybeAssignment() {

  std::vector<AST::Statement::StatementVariant> statements;

  // Both start with a variable declaration.
  auto variable = ParseVariable();
  RET_ON_FAILURE_CODE(
      variable,
      "ParseVaribleAndMaybeAssignment: data type: failed to parse variable",
      lexer);

  // Declare the variable through the analyzer.
  auto declaration = analyzer.variable().ActOnLocalDeclaration(*variable);
  RET_ON_FAILURE_CODE(declaration,
                      "ParseVaribleAndMaybeAssignment: data type: failed to "
                      "act on local variable "
                      "declaration.",
                      lexer);

  statements.push_back(*declaration);

  // Check if the declaration is initalized with an assignment.
  if (lexer.getCurrentToken() == TokenType::EQUAL) {
    auto assignment = ParseAssignment(*variable);

    RET_ON_FAILURE_CODE(
        assignment,
        "ParseVaribleAndMaybeAssignment: data type: failed assignment", lexer);

    // Run the assignment through the analyzer.
    RET_ON_FAILURE_CODE(analyzer.variable().ActOnAssignment(*assignment),
                        "ParseVaribleAndMaybeAssignment: data type: failed to "
                        "act on assignment.",
                        lexer);

    statements.push_back(*assignment);
  }

  // TODO newline should also follow variable declaration without
  // assignment, but grammar currently does not require it.
  RET_ON_WRONG_TOKEN(TokenType::NEWLINE,
                     "ParseVaribleAndMaybeAssignment: data type: newline does "
                     "not follow a variable "
                     "assignment.");

  return statements;
}

Result<AST::Statement::Statement *> Parser::ParseStatement() {
  auto *statement = new AST::Statement::Statement();

  if (lexer.getCurrentToken() == TokenType::RETURN ||
      lexer.getCurrentToken() == TokenType::RIGHT_CURLY_BRACE) {
    return statement;
  }

  int loopCounter = 0;
  skipUntilNotNewline();
  do {
    auto tokenCategory = tokenTypeToCategory.at(lexer.getCurrentToken().type);
    switch (tokenCategory) {

    case TokenCategory::MULTI_PURPOSE: {
      RET_ON_WRONG_TOKEN(TokenType::STAR, "ParseStatement: expected * got &");
      
      int indirectionStep = 0;
      int loopCounter = 0;
      do {
         if (lexer.getCurrentToken() == TokenType::STAR) {
          ++indirectionStep;
        } else {
          break;
        }
        lexer.generateNextToken();
      } while (loopCounter++ < loopLimit);
      
      auto maybeStatement = ParseFunctionCallOrAssignment(indirectionStep);
      RET_ON_FAILURE_CODE(maybeStatement,
                          "ParseStatement: failed function call or assignment.",
                          lexer);

      statement->AddStatement(*maybeStatement);
      break;
    }

    // Variable declaration or assignment.
    case TokenCategory::DATA_TYPE: {

      auto statements = ParseVaribleAndMaybeAssignment();

      RET_ON_FAILURE(statements,
                     "ParseStatement: Failed ParseVaribleAndMaybeAssignment");

      statement->AddStatements(*statements);
      break;
    }

    // ControlFlow.
    case TokenCategory::KEYWORD: {
      // Conditional branch.
      // Always starts with an IF for now.
      RET_ON_WRONG_TOKEN(TokenType::IF, "ParseStatement: keyword: expected if");

      auto controlFlow = ParseConditionalBranchingGroup();
      RET_ON_FAILURE_CODE(controlFlow,
                          "ParseStatement: keyword: failed controlFlow", lexer);

      statement->AddStatement(*controlFlow);
      break;
    }

    // Function call or assignment.
    case TokenCategory::IDENTIFER: {

      auto maybeStatement = ParseFunctionCallOrAssignment();
      RET_ON_FAILURE_CODE(maybeStatement,
                          "ParseStatement: failed function call or assignment.",
                          lexer);

      statement->AddStatement(*maybeStatement);
      break;
    }
    default: {
      return FAILURE_CODE("ParseStatement: unexpected token", lexer);
    }
    }

    // Check if we should stop.
    lexer.generateNextToken();
    skipUntilNotNewline();

    if (lexer.getCurrentToken() == TokenType::RETURN ||
        lexer.getCurrentToken() == TokenType::RIGHT_CURLY_BRACE) {
      break;
    }

  } while (loopCounter++ < loopLimit);
  return statement;
}

} // namespace Parser