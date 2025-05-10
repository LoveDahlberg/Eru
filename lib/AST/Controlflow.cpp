#include <AST/Controlflow.h>
#include <AST/Statement.h>

#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/BasicBlock.h>
#include <llvm/Support/Casting.h>
#include <llvm/IR/Value.h>

namespace AST::Controlflow {

llvm::Value *GenerateComparison(codeGenItems &items,
                                ConditionalBranch *conditionalBranch) {

  // If no expression, then we are in an else branch
  if (conditionalBranch->expression == nullptr) {
    return llvm::ConstantInt::get(
        llvm::Type::getInt1Ty(items.module.getContext()), 1);
  }

  auto result = conditionalBranch->expression->codegen(items);

  llvm::Value *branchResult = nullptr;
  if (result->getType()->isIntegerTy(32)) {
    // If result != 0, then expression is true else its false.
    return items.builder->CreateICmpNE(
        result, llvm::ConstantInt::get(
                    llvm::Type::getInt32Ty(items.module.getContext()), 0));
  }
  return nullptr;
}

llvm::Value *ConditionalBranchingGroup::codegen(codeGenItems &items) {
  if (items.builder == nullptr || items.currentFunction == nullptr) {
    return nullptr;
  }

  auto &context = items.module.getContext();
  auto mergeBlock =
      llvm::BasicBlock::Create(context, "exit", items.currentFunction);

  std::vector<std::pair<llvm::BasicBlock *, llvm::BasicBlock *>> phiNodeTargets;
  int branchNumber;
  for (branchNumber = 0; branchNumber < conditionalChain.size();
       ++branchNumber) {
    auto trueBlock =
        llvm::BasicBlock::Create(context, "then", items.currentFunction);

    auto falseBlock =
        llvm::BasicBlock::Create(context, "else", items.currentFunction);
    auto currentComparisonResult =
        GenerateComparison(items, conditionalChain.at(branchNumber));
    if (currentComparisonResult == nullptr) {
      return nullptr;
    }

    items.builder->CreateCondBr(currentComparisonResult, trueBlock, falseBlock);

    items.builder->SetInsertPoint(trueBlock);

    if (conditionalChain.at(branchNumber)->statement->codegen(items) == nullptr) {
      return nullptr;
    }

    items.builder->CreateBr(mergeBlock);
    items.builder->SetInsertPoint(falseBlock);
  }

  items.builder->SetInsertPoint(mergeBlock);

  auto *Phi = items.builder->CreatePHI(llvm::Type::getInt32Ty(context),
                                       branchNumber, "r");
  return mergeBlock;
}

} // namespace AST::Controlflow