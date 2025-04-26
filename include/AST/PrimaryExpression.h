#pragma once

#include <AST/AST.h>
#include <AST/Controlflow.h>
#include <AST/Declaration.h>
#include <AST/Function.h>
#include <AST/Assignment.h>
#include <AST/Types.h>

namespace AST::PrimaryExpression {

// TODO I prefer not doing it this way, ideally we just use inheritence, but
// that becomes a bit annyoing with circual references.
template <typename T>
concept ValidPrimaryExpressionType =
    std::is_same_v<T, Declaration::VariableDeclaration> ||
    std::is_same_v<T, Assignment::Assignment> ||
    std::is_same_v<T, Controlflow::Controlflow>;

class PrimaryExpression {
public:
  template <typename primaryExpressionConstruct>
    requires ValidPrimaryExpressionType<primaryExpressionConstruct>
  void AddPrimaryExpression(primaryExpressionConstruct *construct) {
    primaryExpressionConstructs.push_back(construct);
  }

private:
  std::vector<AST *> primaryExpressionConstructs;
};

} // namespace AST::PrimaryExpression