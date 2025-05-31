#pragma once

#include <AST/AST.h>
#include <AST/Assignment.h>
#include <AST/Function.h>

namespace AST::Controlflow {

// Common type for all controlflow, to make it easier to group them.
// TODO this could be useful for other AST types as well.
struct Controlflow : public AST {};

struct ConditionalBranch {
  ConditionalBranch(Expression::Expression **expression,
                    Function::Block **block)
      : expression(*expression), block(*block) {}

  ConditionalBranch() : expression(nullptr), block(nullptr) {}

  void addExpression(Expression::Expression **expression) {
    this->expression = *expression;
  }

  void addBlock(Function::Block **block) { this->block = *block; }

  Expression::Expression *expression;
  Function::Block *block;
};

class ConditionalBranchingGroup : public Controlflow {
public:
  ConditionalBranchingGroup(std::vector<ConditionalBranch *> conditionalChain)
      : conditionalChain(conditionalChain) {}

  llvm::Value *codegen(codeGenItems &items) override;

private:
  std::vector<ConditionalBranch *> conditionalChain;
};

} // namespace AST::Controlflow