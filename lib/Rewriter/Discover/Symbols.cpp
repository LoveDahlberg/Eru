

#include <Rewriter/Discover/Symbols.h>
#include <Rewriter/Discover/VirtualAddress.h>

namespace Rewriter {

struct SymbolInfo {
  uint64_t Address;
  llvm::SymbolRef Symbol;
};

struct DiscoveredSymbols {
  std::vector<SymbolInfo> normalSortedSymbols;
  std::vector<llvm::SymbolRef> fileSymbols;
};

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

void RegisterSymbolName(const uint64_t address, const llvm::SymbolRef &symbol,
                        const std::string_view name, const uint64_t finalSize,
                        llvm::bolt::BinaryContext *binaryContext) {

  // Register names even if it's not a function, e.g. for an entry point.
  binaryContext->registerNameAtAddress(
      name, address, llvm::ELFSymbolRef(symbol).getSize(),
      symbol.getAlignment(), cantFail(symbol.getFlags()));
}

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
    const llvm::SymbolRef::Type symbolType = cantFail(symbol.getType());

    // Get a unique symbol name.
    const auto name = [&symbol, &anonymousId]() -> std::string {
      const auto name = cantFail(symbol.getName()).str();

      // If it has no name then simply generate a new unique name.
      if (name.empty()) {
        return "anon." + std::to_string(++anonymousId);
      }

      // If symbol is global then the name has to be unique already.
      if (cantFail(symbol.getFlags()) & llvm::SymbolRef::SF_Global) {
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

    const uint64_t symbolSize = llvm::ELFSymbolRef(symbol).getSize();

    // Check what section the symbol belongs to.
    llvm::section_iterator section = cantFail(symbol.getSection());

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

Result<DiscoveredSymbols> DiscoverSymbols(
    llvm::ELF64LEObjectFile *elfFile, llvm::bolt::BinaryContext *binaryContext,
    std::unordered_map<std::string, uint64_t> &sectionVirtualMapping) {

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

    // Skip the following:
    // - Undefined symbols are not be registered as code, it will be parsed
    //   later.
    // - Other symbols, like section anchor symbols. Not sure when/if we need to
    //   capture this.
    if (flags & SymbolRef::SF_Undefined || flags == SymbolRef::ST_Other) {
      return false;
    }
    if (flags & SymbolRef::SF_Absolute) {
      return true;
    }

    return llvm::bolt::BinarySection(*binaryContext,
                                     *cantFail(Sym.getSection()))
        .isAllocatable();
  };

  for (const SymbolRef &symbol : elfFile->symbols()) {
    const auto type = cantFail(symbol.getType());

    // Catch ST_File symbols exactly as they appear in the table
    if (type == SymbolRef::ST_File) {
      discoveredSymbols.fileSymbols.push_back(symbol);
      continue;
    }

    if (isSymbolInMemory(symbol, type)) {

      auto virtualAddress =
          getVirtualAddress(symbol, elfFile, sectionVirtualMapping);

      discoveredSymbols.normalSortedSymbols.push_back({virtualAddress, symbol});
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
Error DiscoverAndProcessSymbols(
    llvm::ELF64LEObjectFile *elfFile, llvm::bolt::BinaryContext *binaryContext,
    std::unordered_map<std::string, uint64_t> &sectionVirtualMapping) {
  auto maybeSortedSymbols =
      DiscoverSymbols(elfFile, binaryContext, sectionVirtualMapping);
  RET_ON_FAILURE(maybeSortedSymbols, "Failed to discover symbols");

  RET_ON_FAILURE(RegisterSymbols(elfFile, binaryContext, *maybeSortedSymbols),
                 "Failed to register symbols");

  AdjustFunctionBoundaries(binaryContext);

  return ERU_SUCCESS;
}

} // namespace Rewriter