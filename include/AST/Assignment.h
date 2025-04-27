#pragma once

#include <AST/AST.h>
#include <AST/Declaration.h>
#include <AST/Function.h>
#include <AST/Types.h>
#include <AST/Expression.h>

#include <Lexer/Tokens.h>

// stl
#include <variant>

namespace AST::Assignment {

class Assignment : public AST {
public:
  // When assignment is done with declaration.
  Assignment(Declaration::VariableDeclaration *target)
      : target(target) {}

  // When assignment is done on a previously declared variable.
  Assignment(Types::NamedIdentifier target)
      : target(target) {}

  void setExpression(Expression::Expression* expression){
    expression = expression;
  }

  llvm::Value *codegen(llvm::Module &module) override;

private:
  std::variant<Declaration::VariableDeclaration *, Types::NamedIdentifier>
      target;
  Expression::Expression* expression;
};

} // namespace AST::Assignment