#include <Parser/Parser.h>

namespace Parser {

std::optional<AST::VariableDeclaration::Variable *> Parser::ParseVariable() {
  auto type = ParseType();
  if (!type) {
    // err
    return std::nullopt;
  }

  auto identifier = ParseIdentifier();
  if (!identifier) {
    // err
    return std::nullopt;
  }

  return new AST::VariableDeclaration::Variable(*type, *identifier);
}
} // namespace Parser
