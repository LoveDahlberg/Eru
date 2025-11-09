#pragma once

#include "AST/Types.h"
#include "Support/Scope.h"
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
  Block(Statement::Statement *statement, Support::Scope::scopeKind scopeKind)
      : statement(statement), returnValue(nullptr), scopeKind(scopeKind) {}

  Block(Statement::Statement *statement, Expression::Expression *returnValue,
        Support::Scope::scopeKind scopeKind)
      : statement(statement), returnValue(returnValue), scopeKind(scopeKind) {}

  void addReturn(Expression::Expression *returnValue) {
    this->returnValue = returnValue;
  }

  Statement::Statement *statement;
  Expression::Expression *returnValue;
  Support::Scope::scopeKind scopeKind;
};

struct FunctionBody {
  FunctionBody(const std::string &functionName, Block *block)
      : block(block), functionName(functionName) {}

  // TODO add directive
  Block *block;
  const std::string functionName;
};

enum FunctionStatus {
  NONE,
  DECLARATION,
  DEFINITION,
};

struct FunctionCall {
  FunctionCall(std::string name,
               std::vector<Expression::Expression *> parameters)
      : name(name), parameters(parameters) {}

  std::string name;
  // TODO: Does this need to be a pointer still? No polymorphism..
  std::vector<Expression::Expression *> parameters;
};

using Parameters = std::vector<VariableDeclaration::Variable *>;

struct FunctionDeclaration {
  FunctionDeclaration() {}

  FunctionDeclaration(Types::Types type, std::string name,
                      Parameters parameters)
      : type(type), name(name), parameters(parameters) {}

  FunctionDeclaration(Types::Types type, std::string name)
      : type(type), name(name) {}

  std::string name;
  Parameters parameters;

  Types::Types type = Types::NONE;
  FunctionStatus definitionStatus = NONE;
};

} // namespace AST::Function