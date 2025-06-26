#pragma once

#include <AST/VariableDeclaration.h>

// stl
#include <vector>

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
struct Block {
  Block(Statement::Statement *statement) : statement(statement) {}

  Block(Statement::Statement *statement, Expression::Expression *returnValue)
      : statement(statement), returnValue(returnValue) {}

  void addReturn(Expression::Expression *returnValue) {
    this->returnValue = returnValue;
  }

  Statement::Statement *statement;
  Expression::Expression *returnValue;
};

struct FunctionBody {
  FunctionBody(Block *block) : block(block) {}

  // TODO add directive
  Block *block;
};

enum FunctionStatus {
  NONE,
  CALL,
  DECLARATION,
  DEFINITION,
};

struct Function {

  // TODO type and name can be passed as a variableDeclaration, if it make sense
  // for IR generation.
  Function(Types::Types type, std::string name,
           std::vector<VariableDeclaration::Variable *> parameters)
      : type(type), name(name), parameters(parameters) {}

  Function(Types::Types type, std::string name) : type(type), name(name) {}


  void addFunctionBody(FunctionBody *body) { this->body = body; }

  std::vector<VariableDeclaration::Variable *> parameters;

  Types::Types type;

  std::string name;
  
  FunctionStatus definitionStatus = NONE;
  FunctionBody *body;
};

struct FunctionCall {
  FunctionCall(std::string name,
               std::vector<Expression::Expression *> parameters)
      : name(name), parameters(parameters) {}

  std::string name;
  // TODO: Does this need to be a pointer still? No polymorphism..
  std::vector<Expression::Expression *> parameters;
};

} // namespace AST::Function