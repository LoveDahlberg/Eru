#pragma once

#include <AST/AST.h>
#include <AST/Declaration.h>

// TODO make sure #include <AST/Assignment.h> is included before the this header
// in cpp file.
namespace AST::Assignment {
class AssignmentExpressionTarget;
}

namespace AST::Function {

class Function : AST {
public:
  Function(Declaration::FunctionDeclaration *declaration) {}

private:
  std::vector<Declaration::VariableDeclaration *> declarations;
};

class FunctionCall : AST {
  FunctionCall(std::string name,
               std::vector<Assignment::AssignmentExpressionTarget*> parameters)
      : name(name), parameters(parameters) {}

  llvm::Value *codegen(llvm::Module &module) override;

private:
  std::string name;
  std::vector<Assignment::AssignmentExpressionTarget*> parameters;
};

} // namespace AST::Function