#include <Parser/Syntax/Statement.h>

#include <Parser/Syntax/VariableDeclaration.h>
#include <Parser/Syntax/Assignment.h>
#include <Parser/Syntax/Identifier.h>
#include <Parser/Syntax/ControlFlow.h>
#include <Parser/Syntax/Function.h>

namespace Parser::Syntax::Statement {

std::optional<statementAST *> ParseStatement(syntaxItems &items) {
  auto statement = new statementAST;

  if (items.lexer.getCurrentToken().type == TokenType::RIGHT_CURLY_BRACE) {
    return statement;
  }

  int loopCounter = 0;
  bool generateNewToken;
  do {
    generateNewToken = true;
    skipUntilNotNewline(items);
    auto tokenCategory =
        tokenTypeToCategory.at(items.lexer.getCurrentToken().type);
    switch (tokenCategory) {

    // Variable declaration or assignment.
    case TokenCategory::DATA_TYPE: {

      // Both start with a variable declaration.
      auto parameterDeclaration = VariableDeclaration::ParseVariableDeclaration(items);
      if (!parameterDeclaration) {
        // err
        return std::nullopt;
      }

      // Assignment
      if (items.lexer.getCurrentToken().type == TokenType::EQUAL) {
        auto assignment = Assignment::ParseAssignment(items, *parameterDeclaration);
        if (!assignment) {
          // err
          return std::nullopt;
        }
        statement->AddStatement(*assignment);
      }
      // Only a declaration without assignment.
      else {
        statement->AddStatement(*parameterDeclaration);
      }

      break;
    }

    // ControlFlow.
    case TokenCategory::KEYWORD: {
      // Conditional branch.
      // Always starts with an IF for now.
      if (items.lexer.getCurrentToken().type != TokenType::IF) {
        // err
        return std::nullopt;
      }

      auto controlFlow = Controlflow::ParseConditionalBranchingGroup(items);
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
      auto identifier = Identifier::ParseIdentifier(items);
      if (!identifier) {
        // err
        return std::nullopt;
      }

      // TODO move this switch case to getter function.
      switch (items.lexer.getCurrentToken().type) {

      // Assignment
      case TokenType::EQUAL: {
        auto assignment = Assignment::ParseAssignment(
            items, new variableDeclarationAST(nullptr, *identifier));
        if (!assignment) {
          // err
          return std::nullopt;
        }
        statement->AddStatement(*assignment);
        break;
      }

      // Function call
      case TokenType::LEFT_PARENTHESIS: {
        auto assignment = Function::ParseFunctionCall(items, *identifier);
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

    if (items.lexer.generateNextToken().type == TokenType::RIGHT_CURLY_BRACE) {
      break;
    }

  } while (loopCounter++ < loopLimit);
  return statement;
}

} // namespace Parser::Syntax::Statement