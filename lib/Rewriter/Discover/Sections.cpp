

#include <Rewriter/Discover/Sections.h>

namespace Rewriter {

Result<std::unordered_map<std::string, uint64_t>>
DiscoverAndRegisterSections(llvm::ELF64LEObjectFile *elfFile,
                            llvm::bolt::BinaryContext *binaryContext) {

  // Assume symbol table is stripped until its section is found.
  binaryContext->IsStripped = true;

  // Mapping of section name to its virtual mapping. This is needed to give
  // sections a unique address, as sections all have offset 0 in an object file.
  std::unordered_map<std::string, uint64_t> sectionVirtualMapping;

  uint64_t nextAvailableAddress = 0x10000;

  for (const llvm::SectionRef &section : elfFile->sections()) {
    auto SectionNameOrErr = section.getName();
    if (auto err = SectionNameOrErr.takeError()) {
      return ERU_FAILURE("Failed to get section name.");
    }
    const auto sectionName = SectionNameOrErr.get();

    // Save the address, size, offset and content of the original text
    // section.
    if (sectionName == binaryContext->getMainCodeSectionName()) {
      binaryContext->OldTextSectionAddress = section.getAddress();
      binaryContext->OldTextSectionSize = section.getSize();

      auto SectionContentsOrErr = section.getContents();
      if (auto err = SectionContentsOrErr.takeError()) {
        return ERU_FAILURE("Failed to get section content.");
      }
      auto SectionContents = SectionContentsOrErr.get();
      binaryContext->OldTextSectionOffset =
          SectionContents.data() - elfFile->getData().data();
    } else if (sectionName == ".rela.text") {
      binaryContext->HasRelocations = true;
    } else if (sectionName == ".symtab") {
      binaryContext->IsStripped = false;
    } else if (sectionName.empty()) {
      continue;
    }

    assert(!sectionVirtualMapping.contains(sectionName.str()) &&
           "Duplicate section.");

    auto alignment = section.getAlignment().value();
    if (alignment > 0) {
      nextAvailableAddress = llvm::alignTo(nextAvailableAddress, alignment);
    }

    // Register the section in the binary context for quick lookup later
    binaryContext->registerSection(section, nextAvailableAddress);

    sectionVirtualMapping.emplace(sectionName.str(), nextAvailableAddress);
    nextAvailableAddress += section.getSize();
  }

  RET_ON_FALSE(binaryContext->HasRelocations,
               "Object file does not have text relocations in '.rela.text'.");
  RET_ON_TRUE(binaryContext->IsStripped,
              "Object file does not have a symbol table in '.symtab'.");

  return sectionVirtualMapping;
}

} // namespace Rewriter