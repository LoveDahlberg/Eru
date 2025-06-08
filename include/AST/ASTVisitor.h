#include "AST/CompilationUnit.h"
#include "AST/VariableDeclaration.h"
#include <AST/Statement.h>

namespace AST {

class ASTVisitor {
public:
  virtual llvm::Value visitStatement(Statement::Statement &AST) = 0;
  virtual llvm::Value visitAssignment(Assignment::Assignment &AST) = 0;
  virtual llvm::Value visitCompilationUnit(CompilationUnit &AST) = 0;
  virtual llvm::Value visitConditionalBranchingGroup(
      Controlflow::ConditionalBranchingGroup &AST) = 0;
  virtual llvm::Value visitExpression(Expression::Expression &AST) = 0;
  virtual llvm::Value
  visitVariableDeclaration(VariableDeclaration::VariableDeclaration &AST) = 0;
};

} // namespace AST