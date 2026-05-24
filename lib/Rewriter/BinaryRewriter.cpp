#include "bolt/Core/MCPlusBuilder.h"
#include <Frontend/BinaryRewriter.h>
#include <Rewriter/Rewriter.h>
#include <Support/Result.h>

// llvm
#include <cstdint>
#include <llvm/Object/ELFObjectFile.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Support/TargetSelect.h>

// bolt
#include <bolt/Passes/BinaryPasses.h>
#include <bolt/Rewrite/RewriteInstance.h>
#include <memory>
#include <optional>
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
};

// Move to private.
std::optional<RelocationInfo>
GetRelocationInfo(const RelocationRef &relocation,
                  const uint32_t relocationType,
                  llvm::ELF64LEObjectFile *elfFile,
                  llvm::bolt::BinaryContext *binaryContext) {

  RelocationInfo info;

  // Below is mostly taken from BOLT without modification. Make sure only to
  // keep object file and x86 relevant parts.

  if (!llvm::bolt::Relocation::isSupported(relocationType)) {
    return std::nullopt;
  }

  auto IsWeakReference = [](const SymbolRef &Symbol) {
    auto SymFlagsOrErr = Symbol.getFlags();
    if (!SymFlagsOrErr)
      return false;
    return (*SymFlagsOrErr & SymbolRef::SF_Undefined) &&
           (*SymFlagsOrErr & SymbolRef::SF_Weak);
  };

  const size_t RelSize = llvm::bolt::Relocation::getSizeForType(relocationType);

  llvm::ErrorOr<uint64_t> Value =
      binaryContext->getUnsignedValueAtAddress(relocation.getOffset(), RelSize);
  assert(Value && "failed to extract relocated value");

  info.extractedValue = llvm::bolt::Relocation::extractValue(
      relocationType, *Value, relocation.getOffset());
  info.addend = GetRelocationAddend(elfFile, relocation);

  const bool IsPCRelative =
      llvm::bolt::Relocation::isPCRelative(relocationType);
  const uint64_t PCRelOffset = IsPCRelative ? relocation.getOffset() : 0;
  bool SkipVerification = false;
  auto SymbolIter = relocation.getSymbol();
  if (SymbolIter == elfFile->symbol_end()) {
    info.symbolAddress = ExtractedValue - Addend + PCRelOffset;
    MCSymbol *RelSymbol =
        BC->getOrCreateGlobalSymbol(SymbolAddress, "RELSYMat");
    SymbolName = std::string(RelSymbol->getName());
    IsSectionRelocation = false;
  } else {
    const SymbolRef &Symbol = *SymbolIter;
    SymbolName = std::string(cantFail(Symbol.getName()));
    SymbolAddress = cantFail(Symbol.getAddress());
    SkipVerification = (cantFail(Symbol.getType()) == SymbolRef::ST_Other);
    // Section symbols are marked as ST_Debug.
    IsSectionRelocation = (cantFail(Symbol.getType()) == SymbolRef::ST_Debug);
    // Check for PLT entry registered with symbol name
    if (!SymbolAddress && !IsWeakReference(Symbol) &&
        (IsAArch64 || BC->isRISCV())) {
      const BinaryData *BD = BC->getPLTBinaryDataByName(SymbolName);
      SymbolAddress = BD ? BD->getAddress() : 0;
    }
  }

  if (IsSectionRelocation) {
    ErrorOr<BinarySection &> Section = BC->getSectionForAddress(SymbolAddress);
    assert(Section && "section expected for section relocation");
    SymbolName = "section " + std::string(Section->getName());
    // Convert section symbol relocations to regular relocations inside
    // non-section symbols.
    if (Section->containsAddress(ExtractedValue) && !IsPCRelative) {
      SymbolAddress = ExtractedValue;
      Addend = 0;
    } else {
      Addend = ExtractedValue - (SymbolAddress - PCRelOffset);
    }
  }

  auto verifyExtractedValue = [&]() {
    if (SkipVerification)
      return true;

    if (RType == ELF::R_X86_64_PLT32)
      return true;

    return truncateToSize(ExtractedValue, RelSize) ==
           truncateToSize(SymbolAddress + Addend - PCRelOffset, RelSize);
  };

  (void)verifyExtractedValue;
  assert(verifyExtractedValue() && "mismatched extracted relocation value");

  // Inlcude the following
  // targetaddress = symbolAddress + addend
  // From what I understand, this is the target data the relocation points to.
  return {};
}

// In its own file privated with ProcessRelocations
bool HandleRelocations(const SectionRef &relocationTargetSection,
                       const RelocationRef &relocation,
                       llvm::ELF64LEObjectFile *elfFile,
                       llvm::bolt::BinaryContext *binaryContext) {

  // Sanity check that we are processing x86.
  assert(binaryContext->isX86());

  // Information about the target relocation section
  const bool targetSectionIsCode = relocationTargetSection.isText();
  const bool targetSectionIsWritable =
      llvm::bolt::BinarySection(*binaryContext, relocationTargetSection)
          .isWritable();

  // Only attach relocations that come from code (.text).
  // Data-to-data and data-to-code relocations don't need to be
  // tracked on BinaryFunction — they'll be carried forward as-is
  // from the original .rela sections during emit.
  // TODO when we modify data sections, this needs to be handled.
  if (!targetSectionIsCode) {
    return true;
  }

  // Information about the relocation type
  llvm::SmallString<16> relocationTypeName;
  relocation.getTypeName(relocationTypeName);
  const auto relocationType = llvm::bolt::Relocation::getType(relocation);
  const auto relocationOffset = relocation.getOffset();

  // Skip TLS relocations, no special handling is needed on x86.
  if (llvm::bolt::Relocation::isTLS(relocationType)) {
    return true;
  }

  // Parse and return information regarding the relocation.
  const auto maybeRelocationInfo =
      GetRelocationInfo(relocation, relocationType, elfFile, binaryContext);
  if (!maybeRelocationInfo.has_value()) {
    // This is not good, it means we failed to process the relocation.
    return false;
  }
  const auto relocationInfo = *maybeRelocationInfo;

  // If the relocation target is in code, then this is the function it is in.
  auto *functionHoldingTheRelocation = [&]() -> llvm::bolt::BinaryFunction * {
    if (!targetSectionIsCode) {
      return nullptr;
    }
    auto *target = binaryContext->getBinaryFunctionContainingAddress(
        relocationOffset,
        /*CheckPastEnd*/ false,
        /*UseMaxSize*/ true);
    assert(target && "cannot find function for address in code");
    return target;
  }();

  // If the function that should have the relocation actually doesn't contain
  // the relocation address. This is a bit wired, but it means the relocation
  // is in the padding area of the function.
  if (functionHoldingTheRelocation &&
      !functionHoldingTheRelocation->containsAddress(relocationOffset)) {

    // We need to adjust the size of the function, as the relocation tells us
    // there is code in what we first though was padding.
    functionHoldingTheRelocation->setSize(
        functionHoldingTheRelocation->getMaxSize());
    functionHoldingTheRelocation->setSimple(false);
    return true;
  }

  // The symbol that the relocation references. If the relocation symbol is of
  // type STT_SECTION, then it means that it references an entire section, with
  // its addend representing the offset inside of the section that the
  // relocation points to. If not, then it means that the relocation points
  // directly to a named section local symbol.
  //
  // relocationReferencedLocalSymbol is set to nullptr if the relocation points
  // to another section or the target symbol if it is local.
  auto *relocationReferencedLocalSymbol = [&]() -> llvm::MCSymbol * {
    // Relocation target symbol is not local.
    if (relocationInfo.isSectionRelocation) {
      return nullptr;
    }

    // First try to get symbol through the relocation symbol name.
    if (auto *binaryData =
            binaryContext->getBinaryDataByName(relocationInfo.symbolName)) {
      return binaryData->getSymbol();
    }

    // Then try to get the symbol through the relocation symbol address.
    if (auto *binaryData = binaryContext->getBinaryDataAtAddress(
            relocationInfo.symbolAddress)) {
      return binaryData->getSymbol();
    }

    // Cannot find symbol for local relocation.
    return nullptr;
  }();

  // TODO we also need to handle
  // 1. When a relocation refers to a function
  //    -   BinaryFunction *ReferencedBF =
  //          BC->getBinaryFunctionContainingAddress(
  //            Address, /*CheckPastEnd*/ true, /*UseMaxSize*/ false);
  //    -   Here we need to adjust the point of reference to a code location
  //        inside a function, if we see it.
  // 2. When a relocation refers to a section
  //    -   Find the correct referenced symbol inside the section. This is not
  //        handled at all.
  //    -   For now, we only modify the .text section, so the data section is
  //    not needed to touch.

  // At this point we know the relocation is inside a function body.
  // Attach it so the emit stage knows to re-emit it with the
  // updated offset after instruction layout.
  functionHoldingTheRelocation->addRelocation(
      relocationOffset, relocationReferencedLocalSymbol, relocationType,
      relocationInfo.addend, relocationInfo.extractedValue);

  return true;
}

// In its own file privated
void ProcessRelocations(llvm::ELF64LEObjectFile *elfFile,
                        llvm::bolt::BinaryContext *binaryContext) {
  if (!binaryContext->HasRelocations) {
    return;
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
      // TODO propagate errors.
      HandleRelocations(relocationTargetSection, relocation, elfFile,
                        binaryContext);
    }
  }
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

  // 2. processRelocations
  // parse .rela.text and attach each relocation entry to the BinaryFunction
  // that owns the address it applies to. This is important because when you
  // emit your rewritten function bodies, you need to know which relocations
  // to carry forward (references to external symbols) and which ones become
  // internal to your emitted code (branches within the function, which you'll
  // recompute from your CFG).
  ProcessRelocations(elfFile, binaryContext);

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

// Should be private in its own file with DiscoverAndRegisterSymbols
// todo clean this up in to several smaller meaningful functions.
Error RegisterSymbols(llvm::ELF64LEObjectFile *elfFile,
                      llvm::bolt::BinaryContext *binaryContext,
                      const std::vector<SymbolInfo> &sortedSymbols) {

  // Count used to generate unique names for functions without one.
  unsigned anonymousId = 0;

  // The previously added function. As the symbols are sorted, we need to
  // consider if non ST_Function symbols are a part of this function.
  llvm::bolt::BinaryFunction *previousFunction = nullptr;

  for (const auto &[address, symbol] : sortedSymbols) {
    const SymbolRef::Type symbolType = cantFail(symbol.getType());

    // File symbols are skipped.
    if (symbolType == llvm::SymbolRef::ST_File || address == 0) {
      continue;
    }

    // Get a unique symbol name.
    const auto name = [&symbol, &anonymousId]() -> std::string {
      const auto name = cantFail(symbol.getName()).str();
      return name.empty() ? "anon." + std::to_string(++anonymousId) : name;
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
    // function. Means we have found a weak/strong alternative or an alias.
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

      // TODO make addAlternativeName public.
      // newBinaryFunction->addAlternativeName(name);
    } else {
      // Function is new and unique.

      llvm::ErrorOr<llvm::bolt::BinarySection &> binarySection =
          binaryContext->getSectionForAddress(address);

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
Result<std::vector<SymbolInfo>>
DiscoverSymbols(llvm::ELF64LEObjectFile *elfFile,
                llvm::bolt::BinaryContext *binaryContext) {

  // Sort and merge symbols and register them.
  // For each function symbol in the text section, create a BinaryFunction
  // shell object with a name, start address, and size. The size is
  // determined either from st_size in the symbol table entry

  std::vector<SymbolInfo> SortedSymbols;

  // Whether or not a symbol has a meaningful runtime presence. If it is, then
  // we need to save it.
  auto isSymbolInMemory = [&binaryContext](const SymbolRef &Sym) {
    const auto flags = cantFail(Sym.getFlags());
    const auto type = cantFail(Sym.getType());

    if (type & SymbolRef::ST_File || flags & SymbolRef::SF_Undefined) {
      return false;
    }
    if (flags & SymbolRef::SF_Absolute) {
      return true;
    }

    llvm::bolt::BinarySection Section(*binaryContext,
                                      *cantFail(Sym.getSection()));
    return Section.isAllocatable();
  };

  for (const SymbolRef &Symbol : elfFile->symbols()) {
    if (isSymbolInMemory(Symbol)) {
      SortedSymbols.push_back({cantFail(Symbol.getAddress()), Symbol});
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
  llvm::stable_sort(SortedSymbols, CompareSymbols);

  return SortedSymbols;
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

/// TODO this should be an act on object file so that we can later have act
/// on binary.
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
  //  - ELFObjectFileBase must be casted to ELF64LEObjectFile
  llvm::ELF64LEObjectFile *ELF64LEFile =
      llvm::dyn_cast<ELF64LEObjectFile>(objOrErr->getBinary());
  //  - Create binary context.
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
  // - Add MCPlusBuilder as target builder in context
  binaryContext->initializeTarget(std::unique_ptr<llvm::bolt::MCPlusBuilder>(
      llvm::bolt::createX86MCPlusBuilder(
          binaryContext->MIA.get(), binaryContext->MII.get(),
          binaryContext->MRI.get(), binaryContext->STI.get())));

  // 2. Discover the content of the ELF file
  auto result = DiscoverAndRegisterObject(ELF64LEFile, binaryContext.get());
  RET_ON_FAILURE(result, "Failed to discover");

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