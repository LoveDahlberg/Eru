
// bolt
#include <bolt/Core/BinaryContext.h>
#include <bolt/Core/BinaryFunction.h>

#include <Support/Result.h>

namespace Rewriter {

Error DiscoverAndProcessSymbols(
    llvm::ELF64LEObjectFile *elfFile, llvm::bolt::BinaryContext *binaryContext,
    std::unordered_map<std::string, uint64_t> &sectionVirtualMapping);

}