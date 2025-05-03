
#include <Parser/Syntax/Assignment.h>
#include <Parser/Syntax/Expression.h>

namespace Parser::Syntax::Assignment {

  std::optional<assignmentAST *> ParseAssignment(
    syntaxItems &items,
    variableDeclarationAST *variableDeclaration) {

  // If not null, variable information already parsed.
  if (variableDeclaration == nullptr) {
    // TODO implement parsing of variable declaration or single identifier
    return std::nullopt;
  }

  if (items.lexer.getCurrentToken().type != TokenType::EQUAL) {
    // err
    return std::nullopt;
  }

  // Eat the =
  items.lexer.generateNextToken();

  assignmentAST*assignment;
  if (variableDeclaration->type == nullptr) {
    assignment = new assignmentAST({variableDeclaration->name});
  } else {
    assignment = new assignmentAST(variableDeclaration);
  }

  auto expression = Expression::ParseExpression(items);
  if (!expression) {
    // err
    return std::nullopt;
  }

  assignment->setExpression(*expression);

  if (items.lexer.getCurrentToken().type != TokenType::NEWLINE) {
    // err
    return std::nullopt;
  }

  return assignment;
}

}