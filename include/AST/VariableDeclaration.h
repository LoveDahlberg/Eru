#pragma once

#include <optional>
#include <variant>

#include <AST/Types.h>

// TODO move this to expression and fix the recusive include issue.
namespace AST::Expression {
using ConstantOperand =
    std::variant<AST::Types::StringLiteral, AST::Types::IntegerLiteral>;

}
namespace AST::VariableDeclaration {

struct Variable {
  AST::Types::Type type;
  std::string name;
};

struct VariableDeclaration {
  VariableDeclaration(AST::Types::Type type, std::string name)
      : variable(new Variable{type, name}) {}

  VariableDeclaration(Variable *variable, bool isGlobal = false)
      : variable(variable), isGlobal(isGlobal) {}

  Variable *variable;

  bool isGlobal;

  // Set to true if this variable is shadowing one
  bool hidingParentDeclaration = false;
};

struct GlobalVariableInitialization {
  GlobalVariableInitialization(VariableDeclaration *variableDeclaration)
      : variableDeclaration(variableDeclaration) {}

  VariableDeclaration *variableDeclaration;
  std::optional<Expression::ConstantOperand> constOperand;
};

} // namespace AST::VariableDeclaration