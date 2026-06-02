

#include <Rewriter/Discover/Relocations.h>
#include <Rewriter/Discover/VirtualAddress.h>

namespace Rewriter {

int64_t GetRelocationAddend(const llvm::ELF64LEObjectFile *elfFile,
                            const llvm::RelocationRef &RelRef) {

  // Below is mostly taken from BOLT without modification. Make sure only to
  // keep object file and x86 relevant parts.

  using ELFShdrTy = typename llvm::ELF64LE::Shdr;
  using Elf_Rela = typename llvm::ELF64LE::Rela;
  int64_t Addend = 0;
  const llvm::ELFFile<llvm::ELF64LE> &EF = elfFile->getELFFile();
  llvm::DataRefImpl Rel = RelRef.getRawDataRefImpl();
  const ELFShdrTy *RelocationSection = cantFail(EF.getSection(Rel.d.a));
  switch (RelocationSection->sh_type) {
  default:
    llvm_unreachable("unexpected relocation section type");
  case llvm::ELF::SHT_REL:
    break;
  case llvm::ELF::SHT_RELA: {
    const Elf_Rela *RelA = elfFile->getRela(Rel);
    Addend = RelA->r_addend;
    break;
  }
  }

  return Addend;
}

struct RelocationInfo {
  std::string symbolName;
  uint64_t symbolAddress;
  int64_t addend;
  uint64_t extractedValue;
  bool isSectionRelocation;
  uint64_t targetAddress;
  bool isUndefined;
};

Result<uint64_t> GetValueAtOffset(const llvm::bolt::BinarySection *section,
                                  uint64_t offset,
                                  const size_t &sizeOfRelocation) {

  llvm::DataExtractor extractor(section->getContents(),
                                /* IsLittleEndian */ true,
                                /* AddressSize */ 8);

  // Verify the section actually holds enough bytes for this relocation
  RET_ON_TRUE(!extractor.isValidOffsetForDataOfSize(offset, sizeOfRelocation),
              "Relocation offset out of section bounds.");

  return extractor.getUnsigned(&offset, sizeOfRelocation);
}

// Move to private.
Result<RelocationInfo> GetRelocationInfo(
    const llvm::RelocationRef &relocation, const uint32_t relocationType,
    llvm::ELF64LEObjectFile *elfFile, llvm::bolt::BinaryContext *binaryContext,
    llvm::bolt::BinarySection *targetSection,
    std::unordered_map<std::string, uint64_t> &sectionVirtualMapping) {
  RelocationInfo info;

  if (!llvm::bolt::Relocation::isSupported(relocationType)) {
    return {"Relocation type '" + std::to_string(relocationType) +
            "' not supported."};
  }

  // A relocation is in a dedicated section (such as .rela.text for the text
  // section). Each relocation entry contains the following:
  // - Offset: The memory address (offset from the start of the section) within
  //           the target section (so .text for example) where the relocation is
  //           meant to patch.
  // - Info: The relocation type (lower 32 bits) and the index of the target
  //         symbol (upper 32 bits) in the symbol table (for example function
  //         like printf).
  // - Addend: Constant offset. For example if Offset points to usage of
  //           myArray[4], the addend would be 4 elements * 4 bytes = 16 addend.
  // - Extracted value: The data currently present at the offset.
  //
  // We also have:
  //  - Target symbol and symbol address: The address of the symbol the
  //                                      relocation is connected to.
  //  - Target address: This is the final destination the relocation wants to
  //                    read (target symbol address + addend). This needs to be
  //                    calculated so we know what instruction it relates to.
  //
  // For example:
  //
  // clang-format off
// 
//   Assembly from `llvm-objdump -r -l`
//
//       26: 48 8d 3d 00 00 00 00         	leaq	(%rip), %rdi            # 0x2d <main+0x2d>
//       0000000000000029:  R_X86_64_PC32	.L.str-0x4
//       2d: b0 00                        	movb	$0x0, %al
//
//   Symbol table from `readelf -s`
//     
//       Num:    Value          Size Type    Bind   Vis      Ndx Name
//         3: 0000000000000000     9 OBJECT  LOCAL  DEFAULT    4 .L.str
//
//   Section headers with `llvm-objdump --section-headers`
//
//       Sections:
//       Idx Name            Size     VMA              Type
//         4 .rodata.str1.1  00000009 0000000000000000 DATA
//
// The symbol table show that .L.str is in section index 4, which is .rodata.str1.1
//
// - Offset: 0x29, see address first on relocation line
// - Info: Type R_X86_64_PC32 and index 3 in the symbol table.
// - Addend: -0x4. The compiler's adjustment because the RIP register will be at 0x2d when this executes.
// - Extracted value: value at offset 0x29 -> `00 00 00 00`, just passed the `48 8d 3d` leaq.  
// - Target symbol: The actual `L.str` symbol.
// - Symbol address: 0x00 as it is the first byte in Section index 4.
// - Target address: 0x00 calculated as: SymbolAddress (0) + Addend (-4) + PCRelArtifactSize (4) = 0
//
  // clang-format on

  const size_t sizeOfRelocation =
      llvm::bolt::Relocation::getSizeForType(relocationType);

  const auto &offset = relocation.getOffset();

  // Extract the raw value that is written at the offset.
  auto value = GetValueAtOffset(targetSection, offset, sizeOfRelocation);
  RET_ON_FAILURE(value, "Failed to get value at address '" +
                            std::to_string(offset) + "' in section '" +
                            targetSection->getName().str() + "'");

  // Get the data currently present at the offset of the relocation.
  info.extractedValue =
      llvm::bolt::Relocation::extractValue(relocationType, *value, offset);

  info.addend = GetRelocationAddend(elfFile, relocation);

  // Check if relocation is connected to valid symbol. Must always be true for
  // object files.
  auto symbolIter = relocation.getSymbol();
  if (symbolIter == elfFile->symbol_end()) {
    return {"Relocation has no symbol associated with it, object file is "
            "malformed."};
  }

  const llvm::SymbolRef &symbol = *symbolIter;
  info.symbolAddress =
      getVirtualAddress(symbol, elfFile, sectionVirtualMapping);

  // Check whether or not the symbol is undefined. Needed to separate undefined
  // symbols from functions at address 0. The first function of each section
  // will be at address 0, due to all sections being at offset 0 in an object
  // file.
  info.isUndefined =
      (cantFail(symbol.getFlags()) & llvm::SymbolRef::SF_Undefined);

  // Set whether or not the symbol is a section symbol. LLVM assign section
  // symbols to ST_Debug it seems.
  info.isSectionRelocation =
      (cantFail(symbol.getType()) == llvm::SymbolRef::ST_Debug);

  info.symbolName = [&] {
    if (info.isSectionRelocation) {
      llvm::ErrorOr<llvm::bolt::BinarySection &> section =
          binaryContext->getSectionForAddress(info.symbolAddress);
      assert(section && "section expected for section relocation");
      return "section " + std::string(section->getName());
    }
    return std::string(cantFail(symbol.getName()));
  }();

  // Calculate the actual logical memory location the relocation points to.
  // For PC-relative relocations, the compiler bakes in a negative addend to
  // account for the Instruction Pointer advancing. We neutralize that artifact
  // here so the CFG builder looks up the true target address.
  info.targetAddress =
      info.symbolAddress + info.addend +
      (llvm::bolt::Relocation::isPCRelative(relocationType) ? sizeOfRelocation
                                                            : 0);

  return info;
}

llvm::MCSymbol *
GetNonFunctionRelocationSymbol(RelocationInfo &relocationInfo,
                               llvm::bolt::BinaryContext *binaryContext) {

  // Attempt to find the referenced symbol for non section relocations.
  if (!relocationInfo.isSectionRelocation) {

    // Try getting symbol through the relocation symbol name.
    if (auto *binaryData =
            binaryContext->getBinaryDataByName(relocationInfo.symbolName)) {
      return binaryData->getSymbol();
    }

    // Try to get the symbol through the relocation symbol address.
    if (auto *binaryData = binaryContext->getBinaryDataAtAddress(
            relocationInfo.symbolAddress)) {
      return binaryData->getSymbol();
    }
  }

  // If we don't have a symbol and it doesn't point to a known function,
  // we must register it so the relocation has an anchor.

  // When relocation points to somewhere that the linker will fill in. Such
  // as external calls and section relocations.

  // If symbol is undefined
  if (relocationInfo.isUndefined) {
    return binaryContext->getOrCreateUndefinedGlobalSymbol(
        relocationInfo.symbolName);
  }

  // Cant we just do registerNameAtAddress?
  if (relocationInfo.symbolAddress == 0) {
    return binaryContext->Ctx->getOrCreateSymbol(relocationInfo.symbolName);
  }

  // This is likely a absolute data reference.
  return binaryContext->getOrCreateGlobalSymbol(relocationInfo.symbolAddress,
                                                "SYMBOLat");
}

llvm::MCSymbol *GetFunctionRelocationSymbol(
    llvm::bolt::BinaryFunction *relocationTargetFunction,
    llvm::bolt::BinaryFunction *relocationSourceFunction,
    RelocationInfo &relocationInfo, const uint64_t relocationOffset) {

  // The relocation target is not a function.
  if (relocationTargetFunction == nullptr) {
    return nullptr;
  }

  // If the relocation points one byte past the end of the function. This is for
  // example __builtin_unreachable. For this we need to use the function start
  // symbol.
  if (!relocationTargetFunction->containsAddress(relocationInfo.targetAddress,
                                                 /*UseMaxSize*/ true)) {
    return relocationTargetFunction->getSymbol();
  }

  // Calculate the far in the target is in to the function.
  uint64_t refFunctionOffset =
      relocationInfo.targetAddress - relocationTargetFunction->getAddress();

  // We are going to point the relocation symbol directly to the internal
  // function block. So we no longer need the symbol address to be a computation
  // of target address + addend, as we instead just point directly at it.
  // Restore the symbol address and addend that was previously calculated in
  // GetRelocationInfo.
  relocationInfo.symbolAddress = relocationInfo.targetAddress;
  relocationInfo.addend = 0;

  // The symbol points to the function start symbol. Here we just use this exact
  // symbol.
  if (refFunctionOffset <= 0) {
    return relocationTargetFunction->getSymbol();
  }

  // If the target address is a constant island, we need to treat it as data.
  // Constant islands are non executable code inside of functions, which should
  // not be treated as an entry point. We only need a new label to keep track of
  // it.
  if (relocationTargetFunction->isInConstantIsland(
          relocationInfo.targetAddress)) {
    return relocationTargetFunction->getOrCreateLocalLabel(
        relocationInfo.targetAddress);
  }

  // The relocation source targets a specific block or label inside another
  // function. For example a jump from one function into the middle of another.
  if (relocationSourceFunction != relocationTargetFunction) {
    return relocationTargetFunction->addEntryPointAtOffset(refFunctionOffset);
  }

  // The relocation source targets a symbol in the same function, for example a
  // loop. Create a local label for it.
  //
  // Important, when handling relocations from data sections, make sure to call
  // registerInternalRefDataRelocation here.
  return relocationTargetFunction->getOrCreateLocalLabel(
      relocationInfo.targetAddress);
}

// In its own file privated with ProcessRelocations
Error HandleRelocation(
    const SectionRef &relocationTargetSection, const RelocationRef &relocation,
    llvm::ELF64LEObjectFile *elfFile, llvm::bolt::BinaryContext *binaryContext,
    std::unordered_map<std::string, uint64_t> &sectionVirtualMapping) {

  const bool targetSectionIsCode = relocationTargetSection.isText();

  // Only attach relocations that come from code (.text).
  // Data-to-data and data-to-code relocations don't need to be
  // tracked on BinaryFunction — they'll be carried forward as-is
  // from the original .rela sections during emit.
  // TODO when we modify data sections, this needs to be handled.
  if (!targetSectionIsCode) {
    return ERU_SUCCESS;
  }

  // Information about the relocation type
  llvm::SmallString<16> relocationTypeName;
  relocation.getTypeName(relocationTypeName);

  // Shift the offset by the virtual section offset.
  const auto relocationOffset =
      relocation.getOffset() +
      sectionVirtualMapping.at(
          cantFail(relocationTargetSection.getName()).str());

  auto relocationType = llvm::bolt::Relocation::getType(relocation);

  // Strip x86 specific linker manipulation flags if present.
  if (relocationType & llvm::ELF::R_X86_64_converted_reloc_bit) {
    relocationType &= ~llvm::ELF::R_X86_64_converted_reloc_bit;
  }

  // Skip TLS relocations, no special handling is needed on x86.
  if (llvm::bolt::Relocation::isTLS(relocationType)) {
    return ERU_SUCCESS;
  }

  auto *targetSection =
      binaryContext->getSectionForSectionRef(relocationTargetSection);
  RET_ON_TRUE(targetSection == nullptr, "Failed to get target BinarySection");

  // Parse and return information regarding the relocation.
  auto maybeRelocationInfo =
      GetRelocationInfo(relocation, relocationType, elfFile, binaryContext,
                        targetSection, sectionVirtualMapping);

  RET_ON_FAILURE(maybeRelocationInfo, "Parsing of relocation info failed");

  auto relocationInfo = *maybeRelocationInfo;

  // The function that the relocation is in. This is the point to be patched so
  // to say. Since we only handle text relocations for now, relocations are
  // always in functions.
  auto *relocationSourceFunction =
      binaryContext->getBinaryFunctionContainingAddress(
          relocationOffset,
          /* CheckPastEnd */ false,
          /* UseMaxSize */ true);

  assert(relocationSourceFunction &&
         "Cannot find function for address in code");

  // If the function that should have the relocation actually doesn't contain
  // the relocation address. This is a bit stange, but it means the relocation
  // is in the padding area of the function and the relocation is invalid.
  if (!relocationSourceFunction->containsAddress(relocationOffset)) {

    // We need to adjust the size of the function, as the relocation tells us
    // there is code in what we first though was padding.
    relocationSourceFunction->setSize(relocationSourceFunction->getMaxSize());
    relocationSourceFunction->setSimple(false);

    return ERU_SUCCESS;
  }

  // The function that the relocation resolves to. This is the final location
  // that the relocationSouceFunction code wants to reach.
  auto *relocationTargetFunction =
      relocationInfo.isUndefined
          ? nullptr
          : binaryContext->getBinaryFunctionContainingAddress(
                relocationInfo.targetAddress,
                /* CheckPastEnd */ true,
                /* UseMaxSize */ false);

  // Get the corrected relocation symbol if it exists in the target function.
  auto *referencedSymbol = GetFunctionRelocationSymbol(
      relocationTargetFunction, relocationSourceFunction, relocationInfo,
      relocationOffset);

  // If no function was found, then the relocation points to data, an external
  // symbol or a section.
  if (referencedSymbol == nullptr) {
    referencedSymbol =
        GetNonFunctionRelocationSymbol(relocationInfo, binaryContext);
  }

  // Attach the relocation to the source function.
  relocationSourceFunction->addRelocation(relocationOffset, referencedSymbol,
                                          relocationType, relocationInfo.addend,
                                          relocationInfo.extractedValue);

  return ERU_SUCCESS;
}

Error ProcessRelocations(
    llvm::ELF64LEObjectFile *elfFile, llvm::bolt::BinaryContext *binaryContext,
    std::unordered_map<std::string, uint64_t> &sectionVirtualMapping) {
  if (!binaryContext->HasRelocations) {
    return ERU_SUCCESS;
  }

  // Here, go through all sections looking for 'rela.text' relocation sections.
  // This section is a metadata section that describe relocations that apply to
  // another section ('.text'). The sh_info field in the section header points
  // to the target section.
  for (const SectionRef &section : elfFile->sections()) {

    // If this is a relocation section, get the section these relocations apply
    // to. (So one relocation section always points to one section? Can it point
    // to data?)
    const auto relocationTargetSectionItr =
        cantFail(section.getRelocatedSection());

    // If the section isnt relocatable or if there is no target section, skip
    // over this section.
    if (relocationTargetSectionItr == elfFile->section_end()) {
      continue;
    }

    const auto relocationTargetSection = *relocationTargetSectionItr;

    // If the target section (that contains the things that need to be relocated
    // so to say) is not allocatable, skip it.
    //
    // A section is allocatable if the section header has a sh_flags value of
    // SHF_ALLOC. This means the section is loaded into memory at runtime
    // (usually '.text'). So we skip if this is not loaded.
    if (!llvm::bolt::BinarySection(*binaryContext, relocationTargetSection)
             .isAllocatable()) {
      continue;
    }

    // If the relocation section itself is allocatable, skip it.
    //
    // This means that this relocation section is loaded into memory at runtime,
    // which means they are dynamic symbols (I think?). We only want to process
    // static relocation sections here (dynamic ones are created by the linker
    // anyway?).
    if (llvm::bolt::BinarySection(*binaryContext, section).isAllocatable()) {
      continue;
    }

    auto relocatenTargetSectionName =
        cantFail(relocationTargetSection.getName());
    const bool SkipRelocs = llvm::StringSwitch<bool>(relocatenTargetSectionName)
                                .Cases({".plt", ".rela.plt", ".got.plt",
                                        ".eh_frame", ".gcc_except_table"},
                                       true)
                                .Default(false);

    // Skip over dynamic relocations
    if (SkipRelocs) {
      continue;
    }

    // Go through each relocation in the valid relocation section.
    for (const auto &relocation : section.relocations()) {
      RET_ON_FAILURE(HandleRelocation(relocationTargetSection, relocation,
                                      elfFile, binaryContext,
                                      sectionVirtualMapping),
                     "Error handling relocations");
    }
  }

  return ERU_SUCCESS;
}

} // namespace Rewriter