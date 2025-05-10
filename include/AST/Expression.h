#pragma once

#include <AST/AST.h>
#include <AST/Function.h>
#include <AST/Types.h>

#include <Lexer/Tokens.h>

// stl
#include <variant>

namespace AST::Expression {

enum class ArithmeticOperator { PLUS, MINUS };
// TODO move this to appropriate parsing function.
const std::unordered_map<Lexing::TokenType, ArithmeticOperator>
    TokenToArithmeticOperator = {
        {Lexing::TokenType::PLUS, ArithmeticOperator::PLUS},
        {Lexing::TokenType::MINUS, ArithmeticOperator::MINUS},
};

enum class BooleanOperator { OR, AND };
// TODO move this to appropriate parsing function.
const std::unordered_map<Lexing::TokenType, BooleanOperator>
    TokenToBooleanOperator = {
        {Lexing::TokenType::AND, BooleanOperator::AND},
        {Lexing::TokenType::OR, BooleanOperator::OR},
};

using Operator = std::variant<ArithmeticOperator, BooleanOperator>;

using Operand = std::variant<Types::NamedIdentifier, Types::StringLiteral,
                             Types::IntegerLiteral, Function::FunctionCall *>;

struct ExpressionUnit {
  // Is nullopt on first .
  std::optional<Operator> operation;

  Operand operand;
};

class Expression : public AST {
public:
  llvm::Value *codegen(codeGenItems& items) override;

  void addExpressionUnit(ExpressionUnit *expressionUnit) {
    ExpressionUnits.push_back(expressionUnit);
  }

private:
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