#pragma once

#include <AST/AST.h>
#include <AST/Declaration.h>
#include <AST/Function.h>
#include <AST/Types.h>

#include <Lexer/Tokens.h>

// stl
#include <variant>

namespace AST::Assignment {

enum class MathematicalOperator { PLUS, MINUS, END};

// TODO move this to appropriate parsing function.
const std::unordered_map<Lexing::TokenType, MathematicalOperator> TokenToMathematicalOperator = {
{ Lexing::TokenType::PLUS, MathematicalOperator::PLUS},
{ Lexing::TokenType::MINUS, MathematicalOperator::MINUS},
};

// TODO move this together with boolean assignment in controlflow.
struct AssignmentExpression {
  AssignmentExpressionTarget* firstTarget;

  /// Equals to END when there are no more operands.
  MathematicalOperator operation;
  
  AssignmentExpression* SecondTarget;
};

struct AssignmentExpressionTarget {
  AssignmentExpressionTarget(Types::NamedIdentifier target)
  : target(target) {}
  AssignmentExpressionTarget(Types::StringLiteral target)
  : target(target) {}
  AssignmentExpressionTarget(Types::IntegerLiteral target)
  : target(target) {}
  AssignmentExpressionTarget(Function::FunctionCall* target)
  : target(target) {}

  std::variant<Types::NamedIdentifier, Types::StringLiteral,
               Types::IntegerLiteral, Function::FunctionCall*>
      target;
};

class Assignment : public AST {
public:
  // When assignment is done with declaration.
  Assignment(Declaration::VariableDeclaration *targetVariable)
      : targetVariable(targetVariable) {}

  // When assignment is done on a previously declared variable.
  Assignment(Types::NamedIdentifier targetVariable)
      : targetVariable(targetVariable) {}

  void setExpression(AssignmentExpression* expression){
    assignmentExpression = expression;
  }

  llvm::Value *codegen(llvm::Module &module) override;

private:
  std::variant<Declaration::VariableDeclaration *, Types::NamedIdentifier>
      targetVariable;
  AssignmentExpression* assignmentExpression;
};

} // namespace AST::Assignment