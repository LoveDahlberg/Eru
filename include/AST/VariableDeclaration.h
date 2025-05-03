#pragma once

#include <AST/AST.h>
#include <AST/Types.h>

namespace AST::VariableDeclaration {

struct VariableDeclaration : public AST {
  VariableDeclaration(llvm::Type *type, std::string name)
      : type(type), name(name) {}

  llvm::Value *codegen(llvm::Module &module) override;

  llvm::Type *type;
  std::string name;
};

} // namespace AST::VariableDeclaration