#pragma once

#include <AST/Function.h>
#include <AST/Types.h>

#include <Lexer/Tokens.h>

// stl
#include <optional>
#include <variant>

namespace AST::Expression {
using OperandKind =
    std::variant<Types::NamedIdentifier, Types::StringLiteral,
                 Types::IntegerLiteral, Function::FunctionCall *>;

enum class OperandIndirection {
  NONE,

  // Represents getting the address of the subject.
  // Think of it as going upwards.
  GET_ADDRESS,

  // Represents getting what the current subject points to.
  // Think of it as going downwards.
  GET_VALUE
};

struct Operand {
  Operand()
      : indirection(OperandIndirection::NONE), steps(0),
        operandKind(OperandKind()) {}
  Operand(OperandKind operandKind)
        : indirection(OperandIndirection::NONE), steps(0),
          operandKind(operandKind) {}
  Operand(OperandIndirection indirection, int steps)
      : indirection(indirection), steps(steps), operandKind(OperandKind()) {}

  Operand setOperandKind(OperandKind kind) {
    operandKind = kind;
    return *this;
  }

  OperandIndirection indirection;
  int steps;
  OperandKind operandKind;
};

struct ExpressionUnit {
  // Is nullopt on first .
  std::optional<Lexing::Operator> operation; // TODO rename this operator.

  Operand operand;
};

struct Expression {
  void addExpressionUnit(ExpressionUnit *expressionUnit) {
    ExpressionUnits.push_back(expressionUnit);
  }

  std::vector<ExpressionUnit *> ExpressionUnits;
  AST::Types::Type evaluatedType;
};

// a + b + c + d

// exp -> exp operator operand | operand

// 1st step. Go down in exp until operand is hit
//  (a) + b + c + d

// Now take operand and go up a level
//  ((a) + b) + c + d
//  (((a) + b) + c) + d
//  ((((a) + b) + c) + d)

} // namespace AST::Expression