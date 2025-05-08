
#include <AST/Statement.h>

// llvm
#include <llvm/IR/IRBuilder.h>

namespace AST::Statement {

llvm::Value *Statement::codegen(codeGenItems &items) {

  if (items.currentFunction == nullptr) {
    return nullptr;
  }

  auto &context = items.module.getContext();
  auto *basicBlock =
      llvm::BasicBlock::Create(context, "statement", items.currentFunction);
  llvm::IRBuilder<> builder(context);
  items.builder = &builder;

  builder.SetInsertPoint(basicBlock);

  for (auto statement : statements) {
    auto start = statement->codegen(items);
    if(start == nullptr){
      return nullptr;
    }
  }
  return basicBlock;
}

} // namespace AST::Statement