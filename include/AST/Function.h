#pragma once

#include <AST/AST.h>
#include <AST/Declaration.h>

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

class FunctionDefinition : public AST {
public:
  FunctionDefinition(Declaration::FunctionDeclaration *declaration,
    Statement::Statement *statement)
      : declaration(declaration), statement(statement) {}

  llvm::Value *codegen(llvm::Module &module) override;

private:
  Declaration::FunctionDeclaration *declaration;
  Statement::Statement *statement;
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