#pragma once

#include <AST/Function.h>
#include <AST/Types.h>

#include <Lexer/Tokens.h>

// stl
#include <variant>
#include <optional>

namespace AST::Expression {

using Operand = std::variant<Types::NamedIdentifier, Types::StringLiteral,
                             Types::IntegerLiteral, Function::FunctionCall *>;

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