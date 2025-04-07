
#pragma once

#include <AST/AST.h>
#include <AST/Types.h>

namespace AST::Declaration {

class Declaration : public AST {
public:
  Declaration(const Types::Type &type, const Types::Identifier &name)
      : type(type), name(name) {}

  llvm::Value *codegen();

private:
  Types::Type type;
  Types::Identifier name;
};

class VariableDeclaration : public Declaration {
public:
  VariableDeclaration(const Types::Type &type, const Types::Identifier &name)
      : Declaration(type, name) {}

  llvm::Value *codegen();
};

class FunctionDeclaration : public Declaration {
public:
  FunctionDeclaration(const Types::Type &type, const Types::Identifier &name,
                      std::vector<Declaration> parameters)
      : Declaration(type, name), parameters(parameters) {}

  FunctionDeclaration(const Types::Type &type, const Types::Identifier &name)
      : Declaration(type, name) {}

  llvm::Value *codegen();

private:
  std::vector<Declaration> parameters;
};

} // namespace AST::Declaration