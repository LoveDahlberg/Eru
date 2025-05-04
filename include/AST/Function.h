#pragma once

#include <AST/AST.h>
#include <AST/VariableDeclaration.h>

// TODO make sure #include <AST/Expression.h> is included before the this header
// in cpp file.
namespace AST::Expression {
class ExpressionUnit;
}

// TODO make sure #include <AST/Statement.h> is included before the this
// header in cpp file.
namespace AST::Statement {
class Statement;
}

namespace AST::Function {

class FunctionBody : public AST {
public:
  FunctionBody(Statement::Statement *statement) : statement(statement) {}

  llvm::Value *codegen(llvm::Module &module) override;

private:
  Statement::Statement *statement;
};

class Function : public AST {
public:
  // TODO type and name can be passed as a variableDeclaration, if it make sense
  // for IR generation.
  Function(llvm::Type *type, std::string name,
           std::vector<VariableDeclaration::Variable *> parameters)
      : type(type), name(name), parameters(parameters) {}

  Function(llvm::Type *type, std::string name) : type(type), name(name) {}

  llvm::Value *codegen(llvm::Module &module) override;

  void addFunctionBody(FunctionBody *body) { this->body = body; }

private:
  std::vector<VariableDeclaration::Variable *> parameters;
  llvm::Type *type;
  std::string name;
  FunctionBody *body;
};

class FunctionCall : public AST {
public:
  FunctionCall(std::string name,
               std::vector<Expression::ExpressionUnit *> parameters)
      : name(name), parameters(parameters) {}

  llvm::Value *codegen(llvm::Module &module) override;

private:
  std::string name;
  std::vector<Expression::ExpressionUnit *> parameters;
};

} // namespace AST::Function