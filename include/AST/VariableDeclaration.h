#pragma once

#include <AST/AST.h>
#include <AST/Types.h>

namespace AST::VariableDeclaration {

struct Variable {
  llvm::Type *type;
  std::string name;
};

struct VariableDeclaration : public AST {
  VariableDeclaration(llvm::Type *type, std::string name)
      : variable(new Variable{type, name}) {}

  VariableDeclaration(Variable* variable) : variable(variable) {}

  llvm::Value *codegen(codeGenItems& items) override;

  Variable* variable;

  // TODO semantic analysis should figure out what IR this should create.
  bool global = false;
};

} // namespace AST::VariableDeclaration