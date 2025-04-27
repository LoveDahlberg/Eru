#pragma once

#include <AST/AST.h>
#include <AST/Assignment.h>
#include <AST/Function.h>

// stl
#include <variant>

// Make sure #include <AST/PrimaryExpression.h> is defined in cpp file before
// this header.
namespace AST::PrimaryExpression {
class PrimaryExpression;
}

namespace AST::Controlflow {

// Common type for all controlflow, to make it easier to group them.
// TODO this could be useful for other AST types as well.
struct Controlflow : public AST {};

enum BooleanOperator { OR, AND, END };

struct BooleanExpression {
  Assignment::AssignmentExpressionTarget *assignmentExpressionTarget;
  BooleanOperator booleanOperator;
  BooleanExpression *booleanExpression;
};

class ConditionalBranch : public AST {
public:
  void addCondition(BooleanExpression *booleanExpression) {
    booleanExpression = booleanExpression;
  }

  void addPrimaryExpression(
      PrimaryExpression::PrimaryExpression *primaryExpression) {
    primaryExpression = primaryExpression;
  }

  llvm::Value *codegen(llvm::Module &module) override;

private:
  BooleanExpression *booleanExpression;
  PrimaryExpression::PrimaryExpression *primaryExpression;
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