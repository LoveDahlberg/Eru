#include <AST/Controlflow.h>

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/Casting.h>

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

  // The merge block is where all branches converge.
  auto mergeBlock =
      llvm::BasicBlock::Create(context, "exit", items.currentFunction);

  // Loop through all branches.
  int branchNumber;
  for (branchNumber = 0; branchNumber < conditionalChain.size();
       ++branchNumber) {

    auto currentComparisonResult =
        GenerateComparison(items, conditionalChain.at(branchNumber));
    if (currentComparisonResult == nullptr) {
      return nullptr;
    }

    auto trueBlock =
        llvm::BasicBlock::Create(context, "then", items.currentFunction);
    auto falseBlock =
        llvm::BasicBlock::Create(context, "else", items.currentFunction);

    items.builder->CreateCondBr(currentComparisonResult, trueBlock, falseBlock);

    // Create the content of the true basic block.
    items.builder->SetInsertPoint(trueBlock);
    if (conditionalChain.at(branchNumber)->block->codegen(items) == nullptr) {
      return nullptr;
    }

    // Check if the current basic block has generated a return.
    // If so, don't create a branch to the merge block.
    // Note that we have to check the builders current block and not the true
    // block directly, as the block can split off into other blocks.
    if (items.builder->GetInsertBlock()->getTerminator() == nullptr) {
      items.builder->CreateBr(mergeBlock);
    }

    // Set insertion point to the false block. This is the start of the next
    // comparison.
    items.builder->SetInsertPoint(falseBlock);
  }

  items.builder->CreateBr(mergeBlock);

  items.builder->SetInsertPoint(mergeBlock);

  // TODO properly implement phiNode
  // auto *Phi = items.builder->CreatePHI(llvm::Type::getInt32Ty(context),
  //                                     branchNumber, "r");
  return mergeBlock;
}

} // namespace AST::Controlflow