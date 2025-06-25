#include <AST/Statement.h>
#include <Parser/Parser.h>

namespace Parser {

Result<AST::Statement::Statement *> Parser::ParseStatement() {
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
      auto parameterDeclaration = ParseVariable();
      RET_ON_FAILURE(parameterDeclaration,
                     "ParseStatement: data type: failed parameter declaration");

      // Assignment
      if (lexer.getCurrentToken().type == TokenType::EQUAL) {
        auto assignment = ParseAssignment(*parameterDeclaration);

        RET_ON_FAILURE(assignment,
                       "ParseStatement: data type: failed assignment");

        statement->AddStatement(*assignment);
      }
      // Only a declaration without assignment.
      else {
        statement->AddStatement(
            new AST::VariableDeclaration::VariableDeclaration(
                *parameterDeclaration));
      }

      break;
    }

    // ControlFlow.
    case TokenCategory::KEYWORD: {
      // Conditional branch.
      // Always starts with an IF for now.
      RET_ON_WRONG_TOKEN(TokenType::IF, "ParseStatement: keyword: expected if");

      auto controlFlow = ParseConditionalBranchingGroup();
      RET_ON_FAILURE(controlFlow,
                     "ParseStatement: keyword: failed controlFlow");

      statement->AddStatement(*controlFlow);
      break;
    }

    // Function call or assignment.
    case TokenCategory::IDENTIFER: {

      // Both start with an identifier.
      auto identifier = ParseIdentifier();
      RET_ON_FAILURE(identifier,
                     "ParseStatement: identifier: failed identifier");

      // TODO move this switch case to getter function.
      switch (lexer.getCurrentToken().type) {

      // Assignment
      case TokenType::EQUAL: {
        auto assignment =
            ParseAssignment(new AST::VariableDeclaration::Variable(
                AST::Types::Types::NONE, *identifier));
        RET_ON_FAILURE(identifier,
                       "ParseStatement: identifier: equal: failed assignment");

        statement->AddStatement(*assignment);
        break;
      }

      // Function call
      case TokenType::LEFT_PARENTHESIS: {
        auto assignment = ParseFunctionCall(*identifier);
        RET_ON_FAILURE(
            assignment,
            "ParseStatement: identifier: left paran: failed assignment");

        statement->AddStatement(*assignment);
        break;
      }

      default: {
        return {"ParseStatement: identifier: unexpected token"};
      }
      }
      break;
    }
    default: {
      return {"ParseStatement: unexpected token"};
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