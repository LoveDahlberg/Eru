#include <AST/ASTTraversal.h>
#include <AST/CompilationUnit.h>
#include <AST/Expression.h>
#include <IR/IRScope.h>

// llvm
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/NoFolder.h>
#include <llvm/IR/Value.h>

using namespace Support::Scope;

namespace IR {

// TODO refactor this to use the Result error return convention.
class IRGenerator : public ASTTraversal<llvm::Value *> {
  llvm::Module &module;
  llvm::IRBuilder<llvm::NoFolder> *builder = nullptr;

  // Handle functions
  Result<llvm::Value *> handle(Assignment::Assignment &AST) override;
  Result<llvm::Value *>
  handle(VariableDeclaration::VariableDeclaration &AST) override;
  Result<llvm::Value *> handle(Function::FunctionCall &AST) override;
  Result<llvm::Value *> handle(Function::Block &AST) override;
  Result<llvm::Value *> handle(Function::FunctionBody &AST) override;
  Result<llvm::Value *> handle(Function::FunctionDeclaration &AST) override;
  Result<llvm::Value *>
  handle(VariableDeclaration::GlobalVariableInitialization &AST) override;

  Result<llvm::Value *>
  handle(Controlflow::ConditionalBranchingGroup &AST) override;
  Result<llvm::Value *>
  GenerateComparison(Controlflow::ConditionalBranch *conditionalBranch);

  Result<llvm::Value *> handle(Expression::Expression &AST) override;
  Result<llvm::Value *> getOperand(Expression::ExpressionUnit *expressionUnit);

  // Support
  Result<llvm::Type *> GetType(Types::Type type);

  IRScopeHandler scopeHandler;

public:
  IRGenerator(llvm::Module &module) : module(module) {}
};

} // namespace IR