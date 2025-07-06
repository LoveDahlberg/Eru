#include <Parser/Parser.h>

namespace Parser {

Result<AST::VariableDeclaration::Variable *> Parser::ParseVariable() {
  auto type = ParseType();
  RET_ON_FAILURE_CODE(type, "ParseVariable: Failed to parse type", lexer);

  auto identifier = ParseIdentifier();
  RET_ON_FAILURE_CODE(identifier, "ParseVariable: Failed to parse identifier", lexer);

  return new AST::VariableDeclaration::Variable(*type, *identifier);
}
} // namespace Parser
