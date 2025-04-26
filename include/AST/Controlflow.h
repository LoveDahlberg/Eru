#pragma once

#include <AST/AST.h>
#include <AST/Function.h>
#include <AST/Assignment.h>

// stl
#include <variant>

// Make sure #include <AST/PrimaryExpression.h> is defined in cpp file before
// this header.
namespace AST::PrimaryExpression {
class PrimaryExpression;
}

namespace AST::Controlflow {

enum BooleanOperator {
  OR,
  AND
};

struct BooleanExpression;

struct booleanExpressionList {
  BooleanOperator booleanOperator;
  BooleanExpression* booleanExpression;
};

struct BooleanExpression {
  Assignment::AssignmentExpression* assignmentExpression;
  
  // Can be nullptr if empty.
  booleanExpressionList* booleanExpressionList;
};

struct ConditionalExpression : AST {
  BooleanExpression booleanExpression;
  PrimaryExpression::PrimaryExpression *primaryExpression;
};

struct ConditionalBranch : AST {
  llvm::Value *codegen(llvm::Module &module) override;

  std::vector<ConditionalExpression *> conditionalChain;
};

struct Controlflow : AST {
  std::variant<Function::FunctionCall *, ConditionalBranch *>
      controlflowVariant;

  llvm::Value *codegen(llvm::Module &module) override;
};

} // namespace AST::Controlflow