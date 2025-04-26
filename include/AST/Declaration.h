#pragma once

#include <AST/AST.h>
#include <AST/Types.h>

namespace AST::Declaration {

struct Declaration : AST {
  Declaration(llvm::Type *type, std::string name)
      : type(type), name(name) {}

  llvm::Type *type;
  std::string name;
};

class VariableDeclaration : public Declaration {
public:
  VariableDeclaration(llvm::Type *type, std::string name)
      : Declaration(type, name) {}

llvm::Value *codegen(llvm::Module& module) override;
};

class FunctionDeclaration : public Declaration {
public:
  FunctionDeclaration(llvm::Type *type, std::string name,
                      std::vector<VariableDeclaration> parameters)
      : Declaration(type, name), parameters(parameters) {}

  FunctionDeclaration(llvm::Type *type, std::string name)
      : Declaration(type, name) {}

  llvm::Value *codegen(llvm::Module &module) override;

private:
  std::vector<VariableDeclaration> parameters;
};

} // namespace AST::Declaration