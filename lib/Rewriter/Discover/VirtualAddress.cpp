

#include <Rewriter/Discover/VirtualAddress.h>

namespace Rewriter {

uint64_t getVirtualAddress(
    const llvm::object::SymbolRef &symbol,
    llvm::object::ELF64LEObjectFile *elfFile,
    std::unordered_map<std::string, uint64_t> &sectionVirtualMapping) {

  const uint64_t originalAddress = cantFail(symbol.getAddress());

  // Symbol belongs to a section, shift its address by the section's
  // virtual memory address.
  auto symbolSectionIter = cantFail(symbol.getSection());
  if (symbolSectionIter != elfFile->section_end()) {

    const auto sectionName = cantFail(symbolSectionIter->getName());
    assert(sectionVirtualMapping.contains(sectionName.str()) &&
           "Unknown section!");

    return originalAddress + sectionVirtualMapping.at(sectionName.str());
  }

  // No section, so use the original address.
  return originalAddress;
}

} // namespace Rewriter