#include <Parser/Parser.h>

// stl
#include <optional>

namespace Parser {

Result<AST::Assignment::Assignment *>
Parser::ParseAssignment(AST::VariableDeclaration::Variable *variable) {

  // If not null, variable information already parsed.
  // TODO implement parsing of variable declaration or single identifier
  RET_ON_EQUAL_CODE(variable, nullptr, "ParseAssignment: variable is null",
                    lexer);

  RET_ON_WRONG_TOKEN(TokenType::EQUAL, "ParseAssignment: expected =");

  // Eat the =
  lexer.generateNextToken();

  AST::Assignment::Assignment *assignment;
  assignment = new AST::Assignment::Assignment(
      new AST::Types::NamedIdentifier{.value = variable->name});

  auto expression = ParseExpression();
  RET_ON_FAILURE_CODE(expression, "ParseAssignment: expression failed", lexer);

  // TODO fix this ugly mess.
  auto *expr = *expression;
  assignment->setExpression(&expr);

  RET_ON_WRONG_TOKEN(TokenType::NEWLINE, "ParseAssignment: newline expected");

  return assignment;
}

} // namespace Parser