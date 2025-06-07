#pragma once

#include <AST/Types.h>

// TODO remove llvm dependecy here.
#include <llvm/IR/Value.h>

namespace AST::VariableDeclaration {

struct Variable {
  llvm::Type *type;
  std::string name;
};

struct VariableDeclaration {
  VariableDeclaration(llvm::Type *type, std::string name)
      : variable(new Variable{type, name}) {}

  VariableDeclaration(Variable* variable) : variable(variable) {}

  Variable* variable;

  // TODO semantic analysis should figure out what IR this should create.
  bool global = false;
};

} // namespace AST::VariableDeclaration