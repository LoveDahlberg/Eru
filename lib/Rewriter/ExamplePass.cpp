
#include <Rewriter/Rewriter.h>

namespace Rewriter {

llvm::Error MyRewritePass::runOnFunctions(llvm::bolt::BinaryContext &BC) {
  for (auto &[addr, func] : BC.getBinaryFunctions()) {
    if (!func.isSimple() || func.isIgnored())
      continue;

    for (auto &bb : func) {
      for (auto &inst : bb) {
        // Inspect/modify instructions here.
        // BC.MIB is your MCInstrBuilder helper for
        // creating/modifying MCInst objects.
        llvm::outs() << "Visiting inst in: " << func.getPrintName() << "\n";
      }
    }
  }
  return llvm::Error::success();
}

} // namespace Rewriter