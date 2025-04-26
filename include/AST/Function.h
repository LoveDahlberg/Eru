#pragma once

#include <AST/AST.h>
#include <AST/Declaration.h>

// TODO make sure #include <AST/Assignment.h> is included before the this header
// in cpp file.
namespace AST::Assignment {
class AssignmentExpressionTarget;
}

// TODO make sure #include <AST/PrimayExpression.h> is included before the this
// header in cpp file.
namespace AST::PrimaryExpression {
class PrimaryExpression;
}

namespace AST::Function {

class FunctionDefinition : public AST {
public:
  FunctionDefinition(Declaration::FunctionDeclaration *declaration,
    PrimaryExpression::PrimaryExpression *primaryExpression)
      : declaration(declaration), primaryExpression(primaryExpression) {}

  llvm::Value *codegen(llvm::Module &module) override;

private:
  Declaration::FunctionDeclaration *declaration;
  PrimaryExpression::PrimaryExpression *primaryExpression;
};

class FunctionCall : public AST {
public:
  FunctionCall(std::string name,
               std::vector<Assignment::AssignmentExpressionTarget *> parameters)
      : name(name), parameters(parameters) {}

  llvm::Value *codegen(llvm::Module &module) override;

private:
  std::string name;
  std::vector<Assignment::AssignmentExpressionTarget *> parameters;
};

} // namespace AST::Function