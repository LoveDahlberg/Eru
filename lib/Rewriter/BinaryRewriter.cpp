#include "bolt/Core/BinaryFunction.h"
#include "bolt/Core/MCPlusBuilder.h"
#include "llvm/MC/MCSymbol.h"
#include <Frontend/BinaryRewriter.h>
#include <Rewriter/Rewriter.h>
#include <Support/Result.h>

// llvm
#include <llvm/Object/ELFObjectFile.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Support/ToolOutputFile.h>

// bolt
#include <bolt/Core/BinaryEmitter.h>
#include <bolt/Passes/BinaryPasses.h>
#include <bolt/Rewrite/RewriteInstance.h>

// std
#include <cstdint>
#include <memory>
#include <string>
#include <string_view>

namespace Frontend::Action {

// Move to private
int64_t GetRelocationAddend(const ELF64LEObjectFile *elfFile,
                            const RelocationRef &RelRef) {

  // Below is mostly taken from BOLT without modification. Make sure only to
  // keep object file and x86 relevant parts.

  using ELFShdrTy = typename ELF64LE::Shdr;
  using Elf_Rela = typename ELF64LE::Rela;
  int64_t Addend = 0;
  const ELFFile<ELF64LE> &EF = elfFile->getELFFile();
  DataRefImpl Rel = RelRef.getRawDataRefImpl();
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
    const RelocationRef &relocation, const uint32_t relocationType,
    llvm::ELF64LEObjectFile *elfFile, llvm::bolt::BinaryContext *binaryContext,
    llvm::bolt::BinarySection *targetSection) {
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

  const SymbolRef &symbol = *symbolIter;
  info.symbolAddress = cantFail(symbol.getAddress());

  // Check whether or not the symbol is undefined. Needed to separate undefined
  // symbols from functions at address 0. The first function of each section
  // will be at address 0, due to all sections being at offset 0 in an object
  // file.
  info.isUndefined = (cantFail(symbol.getFlags()) & SymbolRef::SF_Undefined);

  // Set whether or not the symbol is a section symbol. LLVM assign section
  // symbols to ST_Debug it seems.
  info.isSectionRelocation =
      (cantFail(symbol.getType()) == SymbolRef::ST_Debug);

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
  if (relocationInfo.symbolAddress == 0) {

    // If symbol is undefined
    if (relocationInfo.isUndefined) {
      return binaryContext->getOrCreateUndefinedGlobalSymbol(
          relocationInfo.symbolName);
    }

    return binaryContext->registerNameAtAddress(relocationInfo.symbolName, 0, 0,
                                                1);
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
Error HandleRelocations(const SectionRef &relocationTargetSection,
                        const RelocationRef &relocation,
                        llvm::ELF64LEObjectFile *elfFile,
                        llvm::bolt::BinaryContext *binaryContext) {

  // Sanity check that we are processing x86.
  assert(binaryContext->isX86());

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
  const auto relocationOffset = relocation.getOffset();
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
  auto maybeRelocationInfo = GetRelocationInfo(
      relocation, relocationType, elfFile, binaryContext, targetSection);

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

// In its own file privated
Error ProcessRelocations(llvm::ELF64LEObjectFile *elfFile,
                         llvm::bolt::BinaryContext *binaryContext) {
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
      RET_ON_FAILURE(HandleRelocations(relocationTargetSection, relocation,
                                       elfFile, binaryContext),
                     "Error handling relocations");
    }
  }

  return ERU_SUCCESS;
}

// In its own file private
void AdjustFunctionBoundaries(llvm::bolt::BinaryContext *binaryContext) {

  // Loop through all functions and make sure their sizes are correct. This
  // might not be needed for object files, but I am not sure. In any case,
  // multiple entry funtions might need to be corrected regardless.
  for (auto functionItr = binaryContext->getBinaryFunctions().begin(),
            functionItrEnd = binaryContext->getBinaryFunctions().end();
       functionItr != functionItrEnd; ++functionItr) {

    auto &currentFunction = functionItr->second;

    // Get the following function.
    const auto *nextFunction = [&]() -> llvm::bolt::BinaryFunction * {
      const auto next = std::next(functionItr);
      return next == functionItrEnd ? nullptr : &next->second;
    }();

    // Get the end of the function. This is either the next function or the end
    // of the section. If the next function is after the end of the section,
    // then it means it is another section. In this case, the current function
    // ends at the end of section. If there is no following functions, the same
    // applies.
    auto functionEndMarker = [&]() {
      const auto sectionEnd =
          currentFunction.getOriginSection()->getEndAddress();
      // TODO Note that this does not handle multiple entries that might be
      // between the start and the end. In that case, we'd have to check if
      // there are any entry block labels in between the start and end.
      return nextFunction == nullptr
                 ? sectionEnd
                 : std::min(nextFunction->getAddress(), sectionEnd);
    }();

    const auto fullFunctionSize =
        functionEndMarker - currentFunction.getAddress();

    // Set the newly calculated size as max size. I dont understand the
    // difference between maxsize and size though.
    currentFunction.setMaxSize(fullFunctionSize);

    // If function has no size, set it as the full size.
    if (currentFunction.getSize() == 0) {
      currentFunction.setSize(fullFunctionSize);
    }

    // If the calculated full size is smaller than current size, set the
    // largest.
    if (fullFunctionSize < currentFunction.getSize()) {
      currentFunction.setSimple(false);
      currentFunction.setMaxSize(currentFunction.getSize());
    }
  }
}

Error ProcessSymbols(llvm::ELF64LEObjectFile *elfFile,
                     llvm::bolt::BinaryContext *binaryContext) {

  AdjustFunctionBoundaries(binaryContext);

  RET_ON_FAILURE(ProcessRelocations(elfFile, binaryContext),
                 "Failed to process relocations");

  return ERU_SUCCESS;
}

// Should be private in its own file with DiscoverAndRegisterSymbols
struct SymbolInfo {
  uint64_t Address;
  SymbolRef Symbol;
};

// Should be private in its own file with DiscoverAndRegisterSymbols
void RegisterSymbolName(const uint64_t address, const SymbolRef &symbol,
                        const std::string_view name, const uint64_t finalSize,
                        llvm::bolt::BinaryContext *binaryContext) {

  // Register names even if it's not a function, e.g. for an entry point.
  binaryContext->registerNameAtAddress(
      name, address, ELFSymbolRef(symbol).getSize(), symbol.getAlignment(),
      cantFail(symbol.getFlags()));
}

struct DiscoveredSymbols {
  std::vector<SymbolInfo> normalSortedSymbols;
  std::vector<SymbolRef> fileSymbols;
};

// Should be private in its own file with DiscoverAndRegisterSymbols
// TODO clean this up in to several smaller meaningful functions.
Error RegisterSymbols(llvm::ELF64LEObjectFile *elfFile,
                      llvm::bolt::BinaryContext *binaryContext,
                      const DiscoveredSymbols &sortedSymbols) {

  // TODO this will break once we have several sections to parse (or even
  // earlier even), as object files have all its sections at address 0. We would
  // have to either work around this or assign virtual section addresses
  // internally.

  // Count used to generate unique names for functions without one.
  unsigned anonymousId = 0;

  // The previously added function. As the symbols are sorted, we need to
  // consider if non ST_Function symbols are a part of this function.
  llvm::bolt::BinaryFunction *previousFunction = nullptr;

  // Go through all the sorted normal symbols.
  for (const auto &[address, symbol] : sortedSymbols.normalSortedSymbols) {
    const SymbolRef::Type symbolType = cantFail(symbol.getType());

    // Get a unique symbol name.
    const auto name = [&symbol, &anonymousId]() -> std::string {
      const auto name = cantFail(symbol.getName()).str();

      // If it has no name then simply generate a new unique name.
      if (name.empty()) {
        return "anon." + std::to_string(++anonymousId);
      }

      // If symbol is global then the name has to be unique already.
      if (cantFail(symbol.getFlags()) & SymbolRef::SF_Global) {
        return name;
      }

      // If we have a local name, then it might not be unique cross object
      // files.
      //
      // For now, we just handle a single object file at the time, so return the
      // name used. TODO If we ever need it in the future, we must use the
      // fileSymbols to find duplicates and name correctly.
      return name;
    }();

    const uint64_t symbolSize = ELFSymbolRef(symbol).getSize();

    // Check what section the symbol belongs to.
    section_iterator section = cantFail(symbol.getSection());

    // Symbol is absolute, i.e. it has no section. Register it.
    if (section == elfFile->section_end()) {
      RegisterSymbolName(address, symbol, name, symbolSize, binaryContext);
      continue;
    }

    // The symbol is a end of section symbol. Register it.
    if (address == section->getAddress() + section->getSize()) {
      RegisterSymbolName(address, symbol, name, symbolSize, binaryContext);
      continue;
    }

    // Symbol is non-text or virtual. Register it.
    if (!section->isText() || section->isVirtual()) {
      RegisterSymbolName(address, symbol, name, symbolSize, binaryContext);
      continue;
    }

    // Check if the symbol actually belongs to the previous function.
    if (previousFunction) {

      const bool addressInPreviousFunction =
          previousFunction->containsAddress(address);
      const bool symbolLocalInPreviousFunction =
          previousFunction->isSymbolValidInScope(symbol, symbolSize) &&
          previousFunction->getSize() == 0;

      // If the symbol is not a function and can be found inside the previous
      // function, then register it as is. Can be for exampel a label.
      if (symbolType != SymbolRef::ST_Function &&
          (addressInPreviousFunction || symbolLocalInPreviousFunction)) {
        RegisterSymbolName(address, symbol, name, symbolSize, binaryContext);
        continue;
      }

      // If the symbols address is inside of the previous function, but isn't
      // the function itself.
      if (addressInPreviousFunction &&
          previousFunction->getAddress() != address) {

        // If the symbol is not valid in the previous functions scope (but is
        // inside of it) then it means we have found an alternative entry.
        if (!previousFunction->isSymbolValidInScope(symbol, symbolSize)) {

          // Register it as size 0.
          RegisterSymbolName(address, symbol, name, 0, binaryContext);

          // Register entrypoint. Offset here is relative to the function
          // start.
          previousFunction->addEntryPointAtOffset(
              address - previousFunction->getAddress());
          continue;
        }

        // Symbol is a valid inside of the previous function, so we register
        // it as a non alternative entry.
        RegisterSymbolName(address, symbol, name, symbolSize, binaryContext);
        continue;
      }
    }

    // The new Binary function to add.
    llvm::bolt::BinaryFunction *newBinaryFunction = nullptr;

    // First, check if we have already added this address as a binary
    // function. If so, then it means we have found a weak/strong alternative or
    // an alias.
    auto iterator = binaryContext->getBinaryFunctions().find(address);
    if (iterator != binaryContext->getBinaryFunctions().end()) {
      newBinaryFunction = &iterator->second;

      // If the size of the current symbol doesn't match the already
      // registered function, set the function to the biggest one to be safe.
      if (symbolSize != newBinaryFunction->getSize()) {

        const auto newSize = std::max(symbolSize, newBinaryFunction->getSize());

        newBinaryFunction->setSize(newSize);
        binaryContext->setBinaryDataSize(address, newSize);
      }

      newBinaryFunction->addAlternativeName(name);
    } else {

      // Function is new and unique.
      auto *binarySection = binaryContext->getSectionForSectionRef(*section);

      // Skip symbols from invalid or zero-sized sections.
      if (!binarySection || !binarySection->getSize()) {
        continue;
      }

      newBinaryFunction = binaryContext->createBinaryFunction(
          name, *binarySection, address, symbolSize);
    }

    RegisterSymbolName(address, symbol, name, symbolSize, binaryContext);
    previousFunction = newBinaryFunction;
  }

  return ERU_SUCCESS;
}

// Should be private in its own file with DiscoverAndRegisterSymbols
Result<DiscoveredSymbols>
DiscoverSymbols(llvm::ELF64LEObjectFile *elfFile,
                llvm::bolt::BinaryContext *binaryContext) {

  // Sort, merge symbols and register them.
  // For each function symbol in the text section, create a BinaryFunction
  // shell object with a name, start address, and size. The size is
  // determined either from st_size in the symbol table entry

  DiscoveredSymbols discoveredSymbols;

  // Whether or not a symbol has a meaningful runtime presence. If it is, then
  // we need to save it.
  auto isSymbolInMemory = [&binaryContext](const SymbolRef &Sym,
                                           const SymbolRef::Type &type) {
    const auto flags = cantFail(Sym.getFlags());

    if (flags & SymbolRef::SF_Undefined) {
      return false;
    }
    if (flags & SymbolRef::SF_Absolute) {
      return true;
    }

    return llvm::bolt::BinarySection(*binaryContext,
                                     *cantFail(Sym.getSection()))
        .isAllocatable();
  };

  for (const SymbolRef &Symbol : elfFile->symbols()) {
    const auto type = cantFail(Symbol.getType());

    // Catch ST_File symbols exactly as they appear in the table
    if (type == SymbolRef::ST_File) {
      discoveredSymbols.fileSymbols.push_back(Symbol);
      continue;
    }

    if (isSymbolInMemory(Symbol, type)) {
      discoveredSymbols.normalSortedSymbols.push_back(
          {cantFail(Symbol.getAddress()), Symbol});
    }
  }

  // Sort symbols.
  auto CompareSymbols = [&binaryContext](const SymbolInfo &A,
                                         const SymbolInfo &B) {
    if (A.Address != B.Address)
      return A.Address < B.Address;

    // Sort function symbols above debug symbols.
    const auto AType = cantFail(A.Symbol.getType());
    const auto BType = cantFail(B.Symbol.getType());
    if (AType == SymbolRef::ST_Function && BType != SymbolRef::ST_Function)
      return true;
    if (BType == SymbolRef::ST_Debug && AType != SymbolRef::ST_Debug)
      return true;

    return false;
  };
  llvm::stable_sort(discoveredSymbols.normalSortedSymbols, CompareSymbols);

  return discoveredSymbols;
}

// public in its own file
Error DiscoverAndRegisterSymbols(llvm::ELF64LEObjectFile *elfFile,
                                 llvm::bolt::BinaryContext *binaryContext) {
  auto maybeSortedSymbols = DiscoverSymbols(elfFile, binaryContext);
  RET_ON_FAILURE(maybeSortedSymbols, "Failed to discover symbols");

  RET_ON_FAILURE(RegisterSymbols(elfFile, binaryContext, *maybeSortedSymbols),
                 "Failed to register symbols");
  return ERU_SUCCESS;
}

Error DiscoverAndRegisterSections(llvm::ELF64LEObjectFile *elfFile,
                                  llvm::bolt::BinaryContext *binaryContext) {

  // Assume symbol table is stripped until its section is found.
  binaryContext->IsStripped = true;

  for (const SectionRef &Section : elfFile->sections()) {
    auto SectionNameOrErr = Section.getName();
    if (auto err = SectionNameOrErr.takeError()) {
      return ERU_FAILURE("Failed to get section name.");
    }
    auto SectionName = SectionNameOrErr.get();

    // Save the address, size, offset and content of the original text
    // section.
    if (SectionName == binaryContext->getMainCodeSectionName()) {
      binaryContext->OldTextSectionAddress = Section.getAddress();
      binaryContext->OldTextSectionSize = Section.getSize();

      auto SectionContentsOrErr = Section.getContents();
      if (auto err = SectionContentsOrErr.takeError()) {
        return ERU_FAILURE("Failed to get section content.");
      }
      auto SectionContents = SectionContentsOrErr.get();
      binaryContext->OldTextSectionOffset =
          SectionContents.data() - elfFile->getData().data();
    } else if (SectionName == ".rela.text") {
      binaryContext->HasRelocations = true;
    } else if (SectionName == ".symtab") {
      binaryContext->IsStripped = false;
    }
    // Could check for .eh_frame, though not needed for object files. BOLT
    // seems to do this when symbol table is not present.

    // Register the section in the binary context for quick lookup later
    binaryContext->registerSection(Section);
  }

  // can call BC->printSections(BC->outs()). Though this can probably be on
  // debug object act?

  RET_ON_FALSE(binaryContext->HasRelocations,
               "Object file does not have text relocations in '.rela.text'.");
  RET_ON_TRUE(binaryContext->IsStripped,
              "Object file does not have a symbol table in '.symtab'.");

  return ERU_SUCCESS;
}

Error DiscoverAndRegisterObject(llvm::ELF64LEObjectFile *elfFile,
                                llvm::bolt::BinaryContext *binaryContext) {

  RET_ON_FAILURE(DiscoverAndRegisterSections(elfFile, binaryContext),
                 "Failed to discover and register sections");
  RET_ON_FAILURE(DiscoverAndRegisterSymbols(elfFile, binaryContext),
                 "Failed to discover and register symbols");
  RET_ON_FAILURE(ProcessSymbols(elfFile, binaryContext),
                 "Failed to process symbols");

  return ERU_SUCCESS;
}

Error DisassembleAndBuildCFG(llvm::bolt::BinaryContext *binaryContext) {
  for (auto &[_, function] : binaryContext->getBinaryFunctions()) {

    // Skip ignored and pseudo functions that doesnt have code.
    if (function.isPseudo() || function.isIgnored()) {
      continue;
    }

    // Attempt to disassemble.
    if (auto err = function.disassemble()) {
      return {"Failed to disassemble one or more functions. " +
              llvm::toString(std::move(err))};
    }

    // Run necessary post-disassembly fixups.
    function.validateInternalBranches();

    // Skip complex functions that cannot be modified.
    if (!function.isSimple()) {
      continue;
    }

    // Build the CFG.
    if (auto err = function.buildCFG(0)) {
      return {"Failed to build CFG. " + llvm::toString(std::move(err))};
    }

    // Finalize the CFG layout.
    function.postProcessCFG();
  }

  return ERU_SUCCESS;
}

Error EmitObjectFile(llvm::bolt::BinaryContext *binaryContext,
                     const std::string &outputPath) {

  // Open the output file stream
  std::error_code errorCode;
  llvm::ToolOutputFile outFile(outputPath, errorCode, llvm::sys::fs::OF_None);

  RET_ON_TRUE(errorCode, "Cannot open output file: '" + outputPath +
                             "' with error '" + errorCode.message() + "'");

  // Create the LLVM MCStreamer
  std::unique_ptr<llvm::MCStreamer> streamer =
      binaryContext->createStreamer(outFile.os());
  
  for (auto &[_, function] : binaryContext->getBinaryFunctions()) {
    
      // TODO add flag for this and then write to file instead.
    llvm::outs() << "Dumping function: " << function.getPrintName() << "\n";
    function.print(llvm::outs(), "Before Emit");

    // 1. Tell the streamer to write into the .text section
    streamer->switchSection(binaryContext->getTextSection());

    // 2. Emit the main function symbol (e.g. 'main') so the linker can find it
    streamer->emitSymbolAttribute(function.getSymbol(), llvm::MCSA_Global);
    streamer->emitLabel(function.getSymbol());

    // 3. Iterate through the CFG and emit the instructions
    for (llvm::bolt::BinaryBasicBlock *bb : function.getLayout().blocks()) {

      // Emit the block's internal label (.LBB00)
      streamer->emitLabel(bb->getLabel());

      for (llvm::MCInst &inst : *bb) {

        // Strip BOLT's internal tracking data before handing it to standard
        // LLVM
        binaryContext->MIB->stripAnnotations(inst);

        // Emit the raw instruction bytes!
        streamer->emitInstruction(inst, *binaryContext->STI);
      }
    }
  }

  streamer->finish();

  RET_ON_TRUE(streamer->getContext().hadError(),
              "LLVM MCStreamer encountered an error during emission.");

  outFile.keep();

  return ERU_SUCCESS;
}

/// TODO this should be an act on object file so that we can later have act
/// on binary.
/// Better idea, the act on should only be the actual pass I want to do, so the
/// actual modifications to the object. All of this can be in the entry at the
/// bottom.
Error BinaryRewriter::ActOn(const Support::IO::Files &files) {

  // Initialize LLVM targets — required before anything BOLT does
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllDisassemblers();

  auto filePath = files.GetObjectFiles().at(0).string();

  // Open the object file
  auto objOrErr = llvm::object::ObjectFile::createObjectFile(filePath);
  if (llvm::Error Err = objOrErr.takeError()) {
    return ERU_FAILURE("Error creating object file.");
  }

  // 1. Initialize
  llvm::ELF64LEObjectFile *ELF64LEFile =
      llvm::dyn_cast<ELF64LEObjectFile>(objOrErr->getBinary());
  llvm::bolt::Relocation::Arch = ELF64LEFile->makeTriple().getArch();

  // Create binary context.
  auto maybeBC = llvm::bolt::BinaryContext::createBinaryContext(
      ELF64LEFile->makeTriple(),
      std::make_shared<llvm::orc::SymbolStringPool>(), filePath, nullptr, true,
      llvm::DWARFContext::create(
          *ELF64LEFile, llvm::DWARFContext::ProcessDebugRelocations::Ignore,
          nullptr, "", llvm::WithColor::defaultErrorHandler,
          llvm::WithColor::defaultWarningHandler),
      llvm::bolt::JournalingStreams{llvm::outs(), llvm::errs()});
  if (auto err = maybeBC.takeError()) {
    return ERU_FAILURE("Error creating binary context.");
  }
  auto binaryContext = std::move(maybeBC.get());

  // Add MCPlusBuilder as target builder in context
  binaryContext->initializeTarget(std::unique_ptr<llvm::bolt::MCPlusBuilder>(
      llvm::bolt::createX86MCPlusBuilder(
          binaryContext->MIA.get(), binaryContext->MII.get(),
          binaryContext->MRI.get(), binaryContext->STI.get())));

  // 2. Discover the content of the ELF file
  RET_ON_FAILURE(DiscoverAndRegisterObject(ELF64LEFile, binaryContext.get()),
                 "Failed to discover");

  // 3. Dissasemble and build the CFG
  RET_ON_FAILURE(DisassembleAndBuildCFG(binaryContext.get()),
                 "failed to disassemble and build CFG");

  // 4. Run passes

  // 5. Emit
  RET_ON_FAILURE(
      EmitObjectFile(binaryContext.get(), files.getFinalOutputPath()),
      "failed to emit object file");

  return ERU_SUCCESS;
}

} // namespace Frontend::Action

namespace Frontend {

Error Rewrite(Action::BinaryAction *action, const Support::IO::Files &file) {

  // TODO: most likely move all standard steps of rewriting here and then
  // carve out what do with the info (passes etc) into act on.

  // Verify that file:
  // - Format -> ELF X86
  // - Has relocations
  // - Is not stripped

  // auto fileContent =
  // Support::IO::getFileContent(file.GetObjectFiles().at(0));
  // RET_ON_FAILURE(fileContent, "Rewrite failed to get content of file.");

  RET_ON_FAILURE(action->ActOn(file), "Failed Rewrite actOn.");

  return ERU_SUCCESS;
}

} // namespace Frontend