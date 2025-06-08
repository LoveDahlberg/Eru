#pragma once

#include <AST/Types.h>


namespace AST::VariableDeclaration {

struct Variable {
  AST::Types::Types type;
  std::string name;
};

struct VariableDeclaration {
  VariableDeclaration(AST::Types::Types type, std::string name)
      : variable(new Variable{type, name}) {}

  VariableDeclaration(Variable* variable) : variable(variable) {}

  Variable* variable;

  // TODO semantic analysis should figure out what IR this should create.
  bool global = false;
};

} // namespace AST::VariableDeclaration