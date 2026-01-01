#pragma once

#include <AST/Function.h>
#include <AST/Types.h>
#include <AST/VariableDeclaration.h>

#include <Lexer/Tokens.h>

#include <Support/Templates.h>

// stl
#include <variant>

namespace AST::Assignment {

using AssignmentVariant =
    std::variant<VariableDeclaration::VariableDeclaration *,
                 Types::NamedIdentifier *>;

/// Concept that the given AssignmentType is:
/// 1. A pointer.
/// 2. One of the variants in AssignmentVariant.
template <typename T>
concept ValidAssignmentType =
    std::is_pointer_v<T> && is_one_of_variant<T, AssignmentVariant>::value;

struct Assignment {

  /// Create as a VariableDeclaration when assignment is done with declaration.
  /// Create as a NamedIdentifier when assignment is done on a previously
  /// declared variable.
  template <ValidAssignmentType assignmentType>
  Assignment(assignmentType target, int indirectionStep = 0)
      : target(target), indirectionSteps(indirectionStep) {}

  void setExpression(Expression::Expression **expression) {
    this->expression = *expression;
  }

  AssignmentVariant target;
  Expression::Expression *expression;
  int indirectionSteps;
};

} // namespace AST::Assignment