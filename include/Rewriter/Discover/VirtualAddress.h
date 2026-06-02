
// llvm
#include <llvm/Object/ELFObjectFile.h>

namespace Rewriter {

uint64_t getVirtualAddress(
    const llvm::object::SymbolRef &symbol,
    llvm::object::ELF64LEObjectFile *elfFile,
    std::unordered_map<std::string, uint64_t> &sectionVirtualMapping);

}