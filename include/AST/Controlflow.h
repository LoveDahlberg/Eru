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

struct ConditionalBranch {
  ConditionalBranch(Expression::Expression **expression,
                    Statement::Statement **statement)
      : expression(*expression), statement(*statement) {}

  ConditionalBranch() : expression(nullptr), statement(nullptr) {}

  void addExpression(Expression::Expression **expression) {
    this->expression = *expression;
  }

  void addStatement(Statement::Statement **statement) {
    this->statement = *statement;
  }

  Expression::Expression *expression;
  Statement::Statement *statement;
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