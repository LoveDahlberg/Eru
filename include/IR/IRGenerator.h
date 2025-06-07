#include <AST/ASTTraversal.h>
#include <AST/CompilationUnit.h>

// llvm
#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/NoFolder.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Value.h>

namespace IR {

class IRGenerator : public ASTTraversal<llvm::Value *> {
  llvm::Module &module;
  llvm::IRBuilder<llvm::NoFolder> *builder = nullptr;
  llvm::Function *currentFunction = nullptr;

  llvm::Value *handle(Assignment::Assignment &AST) override;
  llvm::Value *handle(VariableDeclaration::VariableDeclaration &AST) override;
  llvm::Value *handle(Function::FunctionCall &AST) override;
  llvm::Value *handle(Function::Block &AST) override;
  llvm::Value *handle(Function::FunctionBody &AST) override;
  llvm::Value *handle(Function::Function &AST) override;

  llvm::Value *handle(Controlflow::ConditionalBranchingGroup &AST) override;
  llvm::Value *
  GenerateComparison(Controlflow::ConditionalBranch *conditionalBranch);

  llvm::Value *handle(Expression::Expression &AST) override;
  llvm::Value *getOperand(Expression::ExpressionUnit *expressionUnit);

public:
  IRGenerator(llvm::Module &module) : module(module) {}
};

} // namespace IR