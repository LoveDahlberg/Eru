#pragma once

#include <AST/AST.h>
#include <AST/Assignment.h>
#include <AST/Controlflow.h>
#include <AST/Declaration.h>
#include <AST/Function.h>
#include <AST/Types.h>

namespace AST::PrimaryExpression {

// TODO I prefer not doing it this way, ideally we just use inheritence, but
// that becomes a bit annyoing with circual references.
template <typename T>
concept ValidPrimaryExpressionType =
    std::is_pointer_v<T> &&
    (std::is_same_v<Declaration::VariableDeclaration,
                    std::remove_pointer_t<T>> ||
     std::is_same_v<Assignment::Assignment, std::remove_pointer_t<T>> ||
     std::is_same_v<Function::FunctionCall, std::remove_pointer_t<T>> ||
     std::is_base_of_v<Controlflow::Controlflow, std::remove_pointer_t<T>>);

struct PrimaryExpression : public GeneratingAST {
  template <typename primaryExpressionConstruct>
    requires ValidPrimaryExpressionType<primaryExpressionConstruct>
  void AddExpression(primaryExpressionConstruct construct) {
    primaryExpressionConstructs.push_back(construct);
  }

  std::vector<llvm::Value *> codegen(llvm::Module &module) override;

private:
  std::vector<AST *> primaryExpressionConstructs;
};

} // namespace AST::PrimaryExpression