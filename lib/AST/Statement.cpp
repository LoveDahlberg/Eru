
#include <AST/Statement.h>

// llvm
#include <llvm/IR/IRBuilder.h>

namespace AST::Statement {

llvm::Value *Statement::codegen(llvm::Module &module) {

  auto &context = module.getContext();
  auto *basicBlock = llvm::BasicBlock::Create(context, "statement");
  llvm::IRBuilder<> builder(context);
  builder.SetInsertPoint(basicBlock);

  for (auto statement : statements) {
    auto start = statement->codegen(module);
    if (start != nullptr) {
      builder.Insert(start);
    }
  }
  return basicBlock;
}

} // namespace AST::Statement