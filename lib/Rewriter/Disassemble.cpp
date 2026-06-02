
// bolt
#include <bolt/Core/BinaryFunction.h>

#include <Rewriter/Disassemble.h>

namespace Rewriter {

Error DisassembleAndBuildCFG(llvm::bolt::BinaryContext *binaryContext) {
  for (auto &[_, function] : binaryContext->getBinaryFunctions()) {

    // Skip ignored and pseudo functions that doesnt have code.
    if (function.isPseudo() || function.isIgnored()) {
      continue;
    }

    if (auto err = function.disassemble()) {
      return {"Failed to disassemble one or more functions. " +
              llvm::toString(std::move(err))};
    }

    function.validateInternalBranches();

    if (!function.isSimple()) {
      continue;
    }

    if (auto err = function.buildCFG(0)) {
      return {"Failed to build CFG. " + llvm::toString(std::move(err))};
    }

    function.postProcessCFG();
  }

  return ERU_SUCCESS;
}

} // namespace Rewriter