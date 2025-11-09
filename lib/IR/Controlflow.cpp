#include <IR/IRGenerator.h>

#include <llvm/IR/BasicBlock.h>
#include <llvm/IR/Constant.h>
#include <llvm/IR/Constants.h>
#include <llvm/IR/Value.h>
#include <llvm/Support/Casting.h>

namespace IR {

Result<llvm::Value *> IRGenerator::GenerateComparison(
    Controlflow::ConditionalBranch *conditionalBranch) {

  // If no expression, then we are in an else branch
  if (conditionalBranch->expression == nullptr) {
    return llvm::ConstantInt::get(llvm::Type::getInt1Ty(module.getContext()),
                                  1);
  }

  auto maybeResult = handle(*conditionalBranch->expression);

  RET_ON_FAILURE(
      maybeResult,
      "IRGenerator: GenerateComparison: failure to handle expression");

  auto result = (*maybeResult);

  if (result->getType()->isIntegerTy(32)) {
    // If result != 0, then expression is true else its false.
    return builder->CreateICmpNE(
        result,
        llvm::ConstantInt::get(llvm::Type::getInt32Ty(module.getContext()), 0));
  }
  llvm::report_fatal_error(
      &"IR Generator GenerateComparison: encountered unimplemented type '"
          [result->getType()->getTypeID()]);
}

Result<llvm::Value *>
IRGenerator::handle(Controlflow::ConditionalBranchingGroup &AST) {

  auto *localScope = scopeHandler.CastCurrentToLocalScope();

  RET_ON_TRUE(builder == nullptr || localScope == nullptr ||
                  localScope->getContextData() == nullptr ||
                  localScope->getContextData()->currentFunction == nullptr,
              "IRGenerator: conditional branching group: builder, scope, "
              "context data or current function missing");

  auto currentFunction = localScope->getContextData()->currentFunction;

  auto &context = module.getContext();

  // The merge block is where all branches converge.
  auto mergeBlock = llvm::BasicBlock::Create(context, "exit", currentFunction);

  // Loop through all branches.
  int branchNumber;
  for (branchNumber = 0; branchNumber < AST.conditionalChain.size();
       ++branchNumber) {

    auto currentComparisonResult =
        GenerateComparison(AST.conditionalChain.at(branchNumber));
    RET_ON_FAILURE(
        currentComparisonResult,
        "IRGenerator: conditional branching group: generate comparison failed");

    auto trueBlock = llvm::BasicBlock::Create(context, "then", currentFunction);
    auto falseBlock =
        llvm::BasicBlock::Create(context, "else", currentFunction);

    builder->CreateCondBr(*currentComparisonResult, trueBlock, falseBlock);

    // Create the content of the true basic block.
    builder->SetInsertPoint(trueBlock);
    RET_ON_FAILURE(
        handle(*AST.conditionalChain.at(branchNumber)->block),
        "IRGenerator: conditional branching group: handle of block failed.");

    // Check if the current basic block has generated a return.
    // If so, don't create a branch to the merge block.
    // Note that we have to check the builders current block and not the true
    // block directly, as the block can split off into other blocks.
    if (builder->GetInsertBlock()->getTerminator() == nullptr) {
      builder->CreateBr(mergeBlock);
    }

    // Set insertion point to the false block. This is the start of the next
    // comparison.
    builder->SetInsertPoint(falseBlock);
  }

  builder->CreateBr(mergeBlock);

  builder->SetInsertPoint(mergeBlock);

  // TODO properly implement phiNode
  // auto *Phi = items.builder->CreatePHI(llvm::Type::getInt32Ty(context),
  //                                     branchNumber, "r");
  return mergeBlock;
}

} // namespace IR