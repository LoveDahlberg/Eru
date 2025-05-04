// Include
#include <Parser/Syntax/VariableDeclaration.h>
#include <Parser/Syntax/Identifier.h>
#include <Parser/Syntax/Type.h>

namespace Parser::Syntax::VariableDeclaration {

std::optional<Variable *>
ParseVariable(syntaxItems &items) {
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
} // namespace Parser::Syntax::Declaration

