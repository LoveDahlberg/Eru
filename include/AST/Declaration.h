
#pragma once

#include <AST/AST.h>
#include <AST/Types.h>

namespace AST::Declaration {

class Declaration : public AST {
public:
  Declaration(const Types::Type &type, const std::string &name)
      : type(type), name(name) {}

  llvm::Value *codegen() { return nullptr; }

private:
  Types::Type type;
  std::string name;
};

class VariableDeclaration : public Declaration {
public:
  VariableDeclaration(const Types::Type &type, const std::string &name)
      : Declaration(type, name) {}

  llvm::Value *codegen() { return nullptr; }
};

class FunctionDeclaration : public Declaration {
public:
  FunctionDeclaration(const Types::Type &type, const std::string &name,
                      std::vector<Declaration> parameters)
      : Declaration(type, name), parameters(parameters) {}

  FunctionDeclaration(const Types::Type &type, const std::string &name)
      : Declaration(type, name) {}

  llvm::Value *codegen() { return nullptr; }

private:
  std::vector<Declaration> parameters;
};

} // namespace AST::Declaration