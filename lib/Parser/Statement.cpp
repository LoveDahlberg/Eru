#include <AST/Statement.h>
#include <Parser/Parser.h>

namespace Parser {

Result<AST::Statement::Statement *> Parser::ParseStatement() {
  auto statement = new AST::Statement::Statement();

  if (lexer.getCurrentToken() == TokenType::RETURN ||
      lexer.getCurrentToken() == TokenType::RIGHT_CURLY_BRACE) {
    return statement;
  }

  int loopCounter = 0;
  skipUntilNotNewline();
  do {
    auto tokenCategory = tokenTypeToCategory.at(lexer.getCurrentToken().type);
    switch (tokenCategory) {

    // Variable declaration or assignment.
    case TokenCategory::DATA_TYPE: {

      // Both start with a variable declaration.
      auto variable = ParseVariable();
      RET_ON_FAILURE_CODE(variable,
                          "ParseStatement: data type: failed to parse variable",
                          lexer);

      // Declare the variable through the analyzer.
      auto declaration = analyzer.variable().ActOnLocalDeclaration(*variable);
      RET_ON_FAILURE_CODE(
          declaration,
          "ParseStatement: data type: failed to act on local variable "
          "declaration.",
          lexer);

      statement->AddStatement(*declaration);

      // Check if the declaration is initalized with an assignment.
      if (lexer.getCurrentToken() == TokenType::EQUAL) {
        auto assignment = ParseAssignment(*variable);

        RET_ON_FAILURE_CODE(
            assignment, "ParseStatement: data type: failed assignment", lexer);

        // Run the assignment through the analyzer.
        RET_ON_FAILURE_CODE(
            analyzer.variable().ActOnAssignment(*assignment),
            "ParseStatement: data type: failed to act on assignment.", lexer);

        // TODO consider if we should add a declaration and a assignment
        // statement on assignment, or if it is enough to just add a assignment
        // (which then auto includes a declaration if ).
        statement->AddStatement(*assignment);
      }

      // TODO newline should also follow variable declaration without
      // assignment, but grammar currently does not require it.
      RET_ON_WRONG_TOKEN(
          TokenType::NEWLINE,
          "ParseStatement: data type: newline does not follow a variable "
          "assignment.");
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

      // Both start with an identifier.
      auto identifier = ParseIdentifier();
      RET_ON_FAILURE_CODE(
          identifier, "ParseStatement: identifier: failed identifier", lexer);

      // TODO move this switch case to getter function.
      switch (lexer.getCurrentToken().type) {

      // Assignment
      case TokenType::EQUAL: {
        auto assignment =
            ParseAssignment(new AST::VariableDeclaration::Variable(
                AST::Types::Types::NONE, *identifier));
        RET_ON_FAILURE_CODE(
          assignment, "ParseStatement: identifier: equal: failed assignment",
            lexer);

        // Run the assignment through the analyzer.
        RET_ON_FAILURE_CODE(
            analyzer.variable().ActOnAssignment(*assignment),
            "ParseStatement: identifier: equal: failed to act on assignment.", lexer);

        statement->AddStatement(*assignment);
        break;
      }

      // Function call.
      case TokenType::LEFT_PARENTHESIS: {
        auto assignment = ParseFunctionCall(*identifier);
        RET_ON_FAILURE_CODE(
            assignment,
            "ParseStatement: identifier: left paran: failed assignment", lexer);

        statement->AddStatement(*assignment);
        break;
      }

      default: {
        return FAILURE_CODE("ParseStatement: identifier: unexpected token",
                            lexer);
      }
      }
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