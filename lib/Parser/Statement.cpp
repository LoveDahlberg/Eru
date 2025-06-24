#include "AST/VariableDeclaration.h"
#include <AST/Statement.h>
#include <Parser/Parser.h>

namespace Parser {

std::optional<AST::Statement::Statement *> Parser::ParseStatement() {
  auto statement = new AST::Statement::Statement();

  if (lexer.getCurrentToken().type == TokenType::RETURN ||
      lexer.getCurrentToken().type == TokenType::RIGHT_CURLY_BRACE) {
    return statement;
  }

  int loopCounter = 0;
  bool generateNewToken;
  do {
    generateNewToken = true;
    skipUntilNotNewline();
    auto tokenCategory = tokenTypeToCategory.at(lexer.getCurrentToken().type);
    switch (tokenCategory) {

    // Variable declaration or assignment.
    case TokenCategory::DATA_TYPE: {

      // Both start with a variable declaration.
      auto parameterDeclaration = *ParseVariable();
      if (!parameterDeclaration) {
        // err
        return std::nullopt;
      }

      // Assignment
      if (lexer.getCurrentToken().type == TokenType::EQUAL) {
        auto assignment = ParseAssignment(parameterDeclaration);
        if (!assignment) {
          // err
          return std::nullopt;
        }
        statement->AddStatement(*assignment);
      }
      // Only a declaration without assignment.
      else {
        statement->AddStatement(
            new AST::VariableDeclaration::VariableDeclaration(
                parameterDeclaration));
      }

      break;
    }

    // ControlFlow.
    case TokenCategory::KEYWORD: {
      // Conditional branch.
      // Always starts with an IF for now.
      if (lexer.getCurrentToken().type != TokenType::IF) {
        // err
        return std::nullopt;
      }

      auto controlFlow = ParseConditionalBranchingGroup();
      if (!controlFlow) {
        // err
        return std::nullopt;
      }

      statement->AddStatement(*controlFlow);
      break;
    }

    // Function call or assignment.
    case TokenCategory::IDENTIFER: {

      // Both start with an identifier.
      auto identifier = ParseIdentifier();
      if (identifier.hasFailed) {
        // err
        return std::nullopt;
      }

      // TODO move this switch case to getter function.
      switch (lexer.getCurrentToken().type) {

      // Assignment
      case TokenType::EQUAL: {
        auto assignment =
            ParseAssignment(new AST::VariableDeclaration::Variable(AST::Types::Types::NONE, *identifier));
        if (!assignment) {
          // err
          return std::nullopt;
        }
        statement->AddStatement(*assignment);
        break;
      }

      // Function call
      case TokenType::LEFT_PARENTHESIS: {
        auto assignment = ParseFunctionCall(*identifier);
        if (!assignment) {
          // err
          return std::nullopt;
        }
        statement->AddStatement(*assignment);
        break;
      }

      default: {
        // err
        return std::nullopt;
      }
      }
      break;
    }
    default: {
      // err
      return std::nullopt;
    }
    }

    // Check if we should stop.
    lexer.generateNextToken();
    if (lexer.getCurrentToken().type == TokenType::RETURN ||
        lexer.getCurrentToken().type == TokenType::RIGHT_CURLY_BRACE) {
      break;
    }

  } while (loopCounter++ < loopLimit);
  return statement;
}

} // namespace Parser