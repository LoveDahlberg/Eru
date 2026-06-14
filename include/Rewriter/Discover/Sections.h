// llvm
#include <llvm/Object/ELFObjectFile.h>

// bolt
#include <bolt/Core/BinaryContext.h>

#include <Support/Result.h>

namespace Rewriter {

Result<std::unordered_map<std::string, uint64_t>>
DiscoverAndRegisterSections(llvm::ELF64LEObjectFile *elfFile,
                            llvm::bolt::BinaryContext *binaryContext);

}