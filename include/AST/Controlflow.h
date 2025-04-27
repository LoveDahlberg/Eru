#pragma once

#include <AST/AST.h>
#include <AST/Assignment.h>
#include <AST/Function.h>

// Make sure #include <AST/Statement.h> is defined in cpp file before
// this header.
namespace AST::Statement {
class Statement;
}

namespace AST::Controlflow {

// Common type for all controlflow, to make it easier to group them.
// TODO this could be useful for other AST types as well.
struct Controlflow : public AST {};

class ConditionalBranch : public AST {
public:
  void addExpression(Expression::Expression *expression) {
    expression = expression;
  }

  void addStatement(
    Statement::Statement *Statement) {
        Statement = Statement;
  }

  llvm::Value *codegen(llvm::Module &module) override;

private:
  Expression::Expression *expression;
  Statement::Statement *statement;
};

class ConditionalBranchingGroup : public Controlflow {
public:
  ConditionalBranchingGroup(std::vector<ConditionalBranch *> conditionalChain)
      : conditionalChain(conditionalChain) {}

  llvm::Value *codegen(llvm::Module &module) override;

private:
  std::vector<ConditionalBranch *> conditionalChain;
};

} // namespace AST::Controlflow