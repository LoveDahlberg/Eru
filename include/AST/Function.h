#pragma once

#include <AST/AST.h>
#include <AST/VariableDeclaration.h>

// TODO make sure #include <AST/Expression.h> is included before the this header
// in cpp file.
namespace AST::Expression {
class Expression;
}

// TODO make sure #include <AST/Statement.h> is included before the this
// header in cpp file.
namespace AST::Statement {
class Statement;
}

namespace AST::Function {

/// TODO this might be better to keep in another file/namespace.
class Block : public AST {
public:
  Block(Statement::Statement *statement) : statement(statement) {}

  Block(Statement::Statement *statement, Expression::Expression *returnValue)
      : statement(statement), returnValue(returnValue) {}

  llvm::Value *codegen(codeGenItems &items) override;

  void addReturn(Expression::Expression *returnValue) {
    this->returnValue = returnValue;
  }

private:
  Statement::Statement *statement;
  Expression::Expression *returnValue;
};

class FunctionBody : public AST {
public:
  FunctionBody(Block *block) : block(block) {}

  llvm::Value *codegen(codeGenItems &items) override;

private:
  // TODO add directive
  Block *block;
};

class Function : public AST {
public:
  // TODO type and name can be passed as a variableDeclaration, if it make sense
  // for IR generation.
  Function(llvm::Type *type, std::string name,
           std::vector<VariableDeclaration::Variable *> parameters)
      : type(type), name(name), parameters(parameters) {}

  Function(llvm::Type *type, std::string name) : type(type), name(name) {}

  llvm::Value *codegen(codeGenItems &items) override;

  void addFunctionBody(FunctionBody *body) { this->body = body; }

private:
  std::vector<VariableDeclaration::Variable *> parameters;

  // TODO might not need this type during codegen, only need to check it during
  // semantic passes.
  llvm::Type *type;

  std::string name;
  FunctionBody *body;
};

class FunctionCall : public AST {
public:
  FunctionCall(std::string name,
               std::vector<Expression::Expression *> parameters)
      : name(name), parameters(parameters) {}

  llvm::Value *codegen(codeGenItems &items) override;

private:
  std::string name;
  std::vector<Expression::Expression *> parameters;
};

} // namespace AST::Function