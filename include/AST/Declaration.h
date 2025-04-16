
#pragma once

#include <llvm/IR/Type.h>
#include <AST/AST.h>
#include <AST/Types.h>

namespace AST::Declaration {

struct Declaration {
  Declaration(llvm::Type *type, std::string name)
      : type(type), name(name) {}

  virtual llvm::Value *codegen(llvm::Module& module) = 0;

  llvm::Type *type;
  std::string name;
};

class VariableDeclaration : public Declaration {
public:
  VariableDeclaration(llvm::Type *type, std::string name)
      : Declaration(type, name) {}

llvm::Value *codegen(llvm::Module& module);
};

class FunctionDeclaration : public Declaration {
public:
  FunctionDeclaration(llvm::Type *type, std::string name,
                      std::vector<VariableDeclaration> parameters)
      : Declaration(type, name), parameters(parameters) {}

  FunctionDeclaration(llvm::Type *type, std::string name)
      : Declaration(type, name) {}

  llvm::Value *codegen(llvm::Module &module);

private:
  std::vector<VariableDeclaration> parameters;
};

} // namespace AST::Declaration