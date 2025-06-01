
#include "Parser/VariableDeclaration.h"
#include <Parser/Assignment.h>
#include <Parser/Expression.h>

namespace Parser::Assignment {

std::optional<assignmentAST *> ParseAssignment(Parser &items,
                                               Variable *variable) {

  // If not null, variable information already parsed.
  if (variable == nullptr) {
    // TODO implement parsing of variable declaration or single identifier
    return std::nullopt;
  }

  if (items.lexer.getCurrentToken().type != TokenType::EQUAL) {
    // err
    return std::nullopt;
  }

  // Eat the =
  items.lexer.generateNextToken();

  assignmentAST *assignment;
  if (variable->type == nullptr) {
    assignment = new assignmentAST({variable->name});
  } else {
    assignment = new assignmentAST(new variableDeclarationAST(variable));
  }

  auto expression = Expression::ParseExpression(items);
  if (!expression) {
    // err
    return std::nullopt;
  }

  assignment->setExpression(&*expression);

  if (items.lexer.getCurrentToken().type != TokenType::NEWLINE) {
    // err
    return std::nullopt;
  }

  return assignment;
}

} // namespace Parser::Assignment