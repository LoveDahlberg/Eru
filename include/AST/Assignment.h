#pragma once

#include <AST/AST.h>
#include <AST/Declaration.h>
#include <AST/Types.h>
#include <AST/Function.h>

// stl
#include <variant>

namespace AST::Assignment {

enum class MathematicalOperator { PLUS, MINUS };

class AssignmentExpression {
  AssignmentExpression *firstTarget;
  MathematicalOperator operation;
  AssignmentExpression *SecondTarget;
};


struct AssignmentExpressionTarget : public AssignmentExpression {
  std::variant<Types::NamedIdentifier, Types::StringLiteral,
               Types::IntegerLiteral, Function::FunctionCall>
      target;
};

class Assignment {
  std::variant<Declaration::VariableDeclaration, Types::NamedIdentifier>
      targetVariable;
  AssignmentExpression assignmentExpression;
};

} // namespace AST::Assignment