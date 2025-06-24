#include <Parser/Parser.h>

namespace Parser {

Result<AST::VariableDeclaration::Variable *> Parser::ParseVariable() {
  auto type = ParseType();
  RET_ON_FAILURE(type, "ParseVariable: Failed to parse type");

  auto identifier = ParseIdentifier();
  RET_ON_FAILURE(identifier, "ParseVariable: Failed to parse identifier");

  return new AST::VariableDeclaration::Variable(*type, *identifier);
}
} // namespace Parser
