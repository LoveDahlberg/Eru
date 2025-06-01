// Include
#include <Parser/VariableDeclaration.h>
#include <Parser/Identifier.h>
#include <Parser/Type.h>

namespace Parser::VariableDeclaration {

std::optional<Variable *>
ParseVariable(Parser &items) {
  auto type = Type::ParseType(items);
  if (!type) {
    // err
    return std::nullopt;
  }

  auto identifier = Identifier::ParseIdentifier(items);
  if (!identifier) {
    // err
    return std::nullopt;
  }

  return new Variable(*type, *identifier);
}
} // namespace Parser::Declaration

