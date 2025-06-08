#include <Parser/Parser.h>

// stl
#include <optional>

namespace Parser {

std::optional<AST::Assignment::Assignment *>
Parser::ParseAssignment(AST::VariableDeclaration::Variable *variable) {

  // If not null, variable information already parsed.
  if (variable == nullptr) {
    // TODO implement parsing of variable declaration or single identifier
    return std::nullopt;
  }

  if (lexer.getCurrentToken().type != TokenType::EQUAL) {
    // err
    return std::nullopt;
  }

  // Eat the =
  lexer.generateNextToken();

  AST::Assignment::Assignment *assignment;
  if (variable->type == AST::Types::Types::NONE) {
    assignment = new AST::Assignment::Assignment(
        new AST::Types::NamedIdentifier{.value = variable->name});
  } else {
    assignment =
        new AST::Assignment::Assignment(new AST::VariableDeclaration::VariableDeclaration(variable));
  }

  auto expression = ParseExpression();
  if (!expression) {
    // err
    return std::nullopt;
  }

  assignment->setExpression(&*expression);

  if (lexer.getCurrentToken().type != TokenType::NEWLINE) {
    // err
    return std::nullopt;
  }

  return assignment;
}

} // namespace Parser