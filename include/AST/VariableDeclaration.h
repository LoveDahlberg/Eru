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

  VariableDeclaration(Variable* variable, bool isGlobal = false) : variable(variable), isGlobal(isGlobal){}

  Variable* variable;

  bool isGlobal;

  // Set to true if this variable is shadowing one 
  bool hidingParentDeclaration = false;
};

} // namespace AST::VariableDeclaration