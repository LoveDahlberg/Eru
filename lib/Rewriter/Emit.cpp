

// bolt
#include "bolt/Core/BinaryData.h"
#include "llvm/MC/MCDirectives.h"
#include "llvm/MC/MCSection.h"
#include <bolt/Core/BinaryFunction.h>

// llvm
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/ToolOutputFile.h>

#include <Rewriter/Emit.h>

#include <ranges>

namespace Rewriter {

llvm::MCSymbolAttr getEmitterFlag(const uint32_t symbolRefFlag) {

  // Map only equivalent flags.
  switch (symbolRefFlag) {
  case llvm::object::SymbolRef::SF_Global:
    return llvm::MCSA_Global;

  case llvm::object::SymbolRef::SF_Weak:
    return llvm::MCSA_Weak;

  case llvm::object::SymbolRef::SF_Indirect:
    return llvm::MCSA_IndirectSymbol;

  case llvm::object::SymbolRef::SF_Exported:
    return llvm::MCSA_Exported;

  case llvm::object::SymbolRef::SF_Hidden:
    return llvm::MCSA_Hidden;

  default:
    return llvm::MCSA_Invalid;
  }
}

void forwardSymbolAttributes(llvm::MCSymbol *symbol,
                             llvm::bolt::BinaryData *originalSymbolData,
                             llvm::MCStreamer *streamer) {

  if (originalSymbolData == nullptr) {
    return;
  }

  // Flags are defined in llvm/include/llvm/Object/SymbolicFile.h
  const uint32_t flags = originalSymbolData->getFlags();

  constexpr auto max_flag_value = 11;
  for (int i = 0; i <= max_flag_value; ++i) {
    const uint32_t currentFlag = 1U << i;

    if (!(flags & currentFlag)) {
      continue;
    }

    auto emitterFlag = getEmitterFlag(currentFlag);
    if (emitterFlag == llvm::MCSA_Invalid) {
      continue;
    }

    streamer->emitSymbolAttribute(symbol, emitterFlag);
  }
}

struct DataAtOffset {
  std::vector<llvm::bolt::BinaryData *> DataSymbols;
  std::vector<llvm::bolt::Relocation> Relocations;
};

void emitDataSections(llvm::bolt::BinaryContext *binaryContext,
                      llvm::MCStreamer *streamer) {
  for (auto &section : binaryContext->sections()) {

    if (!section.isAllocatable() || !section.isData() || section.isText()) {
      continue;
    }

    // Section content is either the original, if unchanged, or the modified
    // version otherwise.
    auto sectionContents = section.isFinalized() ? section.getOutputContents()
                                                 : section.getContents();
    auto *targetELFSection = binaryContext->Ctx->getELFSection(
        section.getName(), section.getELFType(), section.getELFFlags());

    streamer->switchSection(targetELFSection);
    streamer->emitValueToAlignment(section.getAlign());

    // All of the data to emit at each offset (section relative). Each offset
    // point can have multiple labels and relocations.
    std::map<uint64_t, DataAtOffset> dataToEmit;

    for (const auto &[address, binaryData] :
         binaryContext->getBinaryDataForSection(section)) {
      dataToEmit[address - section.getAddress()].DataSymbols.emplace_back(
          binaryData);
    }

    for (const auto &relocation : section.relocations()) {
      dataToEmit[relocation.Offset].Relocations.emplace_back(relocation);
    }

    // If there are no relocations or symbols found, just emit the raw bytes.
    if (dataToEmit.empty()) {
      streamer->emitBytes(sectionContents);
      continue;
    }

    uint64_t currentSectionOffset = 0;
    for (const auto &[offset, data] : dataToEmit) {
      // If there are bytes before the first meaningful data, emit their raw
      // bytes.
      if (currentSectionOffset < offset) {
        streamer->emitBytes(sectionContents.substr(
            currentSectionOffset, offset - currentSectionOffset));
        currentSectionOffset = offset;
      }

      // First emit all symbols as labels with relevant attributes and size
      // information for the symbol table. This does not increment the streamer.
      for (auto *dataSymbol : data.DataSymbols) {
        auto symbol = dataSymbol->getSymbol();

        forwardSymbolAttributes(symbol, dataSymbol, streamer);
        streamer->emitSymbolAttribute(symbol, llvm::MCSA_ELF_TypeObject);

        if (dataSymbol->getSize() > 0) {
          streamer->emitELFSize(
              symbol, llvm::MCConstantExpr::create(dataSymbol->getSize(),
                                                   streamer->getContext()));
        }

        streamer->emitLabel(symbol);
      }

      // Then emit all relocations found at this address.
      if (!data.Relocations.empty()) {
        currentSectionOffset += llvm::bolt::Relocation::emit(
            data.Relocations.begin(), data.Relocations.end(), streamer);
      }
    }

    // If there are bytes left over afterwards, emit them.
    if (currentSectionOffset < sectionContents.size()) {
      streamer->emitBytes(sectionContents.substr(currentSectionOffset));
    }
  }
}

/// Represents a continous range of data inside of a function. This ends
/// either at another chunk of data, which might contain different kind of
/// data and/or at different alignment, or when executable code starts again.
struct IslandChunk {
  /// The start offset where data begins.
  uint64_t startOffset;

  /// The end offset where either code beings or another data block beings.
  uint64_t endOffset;

  /// All jump labels inside of the chunk with a offset and symbol pair.
  std::vector<std::pair<const uint64_t, llvm::MCSymbol *>> jumpLabels;

  /// Relocations inside of the chunk.
  std::vector<llvm::bolt::Relocation> relocations;
};

/// Go through all constant islands in the current function and collect them
/// as island chunks. An island chunk represents a sequential sequence of data
/// that all hold the same kind of data with the same alignment. One chunk may
/// immediately be followed by another one in the original function, but this
/// is still considered a new chunk by LLVM if they are separated by a 'start
/// data' marker.
std::vector<IslandChunk>
BuildIslandChunks(llvm::bolt::BinaryFunction &function) {

  auto &islands = function.getIslandInfo();

  auto getNextBoundaryOrMax = [&](const auto &set,
                                  const uint64_t startOffset) -> uint64_t {
    // Get the range greater to or equal the startOffset.
    auto range = std::ranges::upper_bound(set, startOffset);

    // If offset is too large, set as absolute function end.
    return range != set.end() ? *range : function.getMaxSize();
  };

  auto getMapSubrange = [](const auto &map, const uint64_t startOffset,
                           const uint64_t endOffset) {
    return std::ranges::subrange(map.lower_bound(startOffset),
                                 map.lower_bound(endOffset));
  };

  // Loop through all DataOffsets, which denotes the start of a contiguous
  // block of data in code. This block of data can contain several labels that
  // the code can jump to.
  std::vector<IslandChunk> chunks;
  for (const auto startOffset : islands.DataOffsets) {
    IslandChunk chunk;

    chunk.startOffset = startOffset;

    // Calcuate the end offset of this chunk. The end is either:
    // - The start marker of the nearest other data chunk (which is right next
    //   to our chunk). This is data of different type and/or alignment.
    // - The nearest code start marker. We take which ever is closest.
    //   - This is what is in CodeOffsets, offsets where executable code
    //   starts.
    const auto startOfNextDataBlock =
        getNextBoundaryOrMax(islands.DataOffsets, startOffset);
    const auto endOfCurrentDataBlock =
        getNextBoundaryOrMax(islands.CodeOffsets, startOffset);
    std::min(startOfNextDataBlock, endOfCurrentDataBlock);

    // If there is no data in the range, skip.
    if (chunk.startOffset == chunk.endOffset)
      continue;

    // Get the labels that are inside of our chunk.
    const auto labelSubRange =
        getMapSubrange(islands.Offsets, chunk.startOffset, chunk.endOffset);
    for (const auto &item : labelSubRange) {
      chunk.jumpLabels.push_back(item);
    }

    // Get the relocations that are inside of our chunk.
    const auto relocationSubRange =
        getMapSubrange(islands.Relocations, chunk.startOffset,
                       chunk.endOffset) |
        std::views::values;
    for (const auto &item : relocationSubRange) {
      chunk.relocations.push_back(item);
    }

    chunks.push_back(std::move(chunk));
  }

  return chunks;
}

/// Constant islands represents parts of executable code that is actually
/// data. One function may contain several islands of data through out the
/// functions. These islands of data can all be placed separately, or grouped
/// back to back.
///
/// Here, we gather them all up and emit them in one big island at the end of
/// the current function. This is taken from bolt which does this for I-cache
/// optimization. It doesn't really matter for us here, we just want to emit
/// it for now. Ideally we control the layout of the function explicitly in
/// passes, but for now we can keep this as is.
void emitConstantIsland(llvm::bolt::BinaryContext *binaryContext,
                        llvm::bolt::BinaryFunction &function,
                        llvm::MCStreamer *streamer) {

  auto &islands = function.getIslandInfo();
  if (islands.DataOffsets.empty()) {
    return;
  }

  // Go through all constant islands in the function, that are possible spread
  // out, and store their offset content as a island chunk.
  const std::vector<IslandChunk> islandChunks = BuildIslandChunks(function);

  // Get the raw contents of the function. We are going to use this to emit
  // specific sequences of data.
  const auto FunctionContents =
      function.getOriginSection()->getContents().substr(
          function.getAddress() - function.getOriginSection()->getAddress(),
          function.getMaxSize());

  // TODO, make into a separate function.
  // Emits the relevant data and relocations within a offset range.
  auto emitDataInRange = [&](uint64_t currentOffset,
                             const uint64_t targetEndOffset,
                             const auto relocations) -> uint64_t {
    // Emit all relevant relocations.
    for (const auto &relocation : relocations) {
      // If there is space in between where the current offset and where the
      // relocation starts. This simply means that we have raw data before the
      // actual relocation. Emit this data so that we are at the point where
      // the relocation starts.
      if (currentOffset < relocation.Offset) {
        streamer->emitBytes(
            FunctionContents.slice(currentOffset, relocation.Offset));
        currentOffset = relocation.Offset;
      }

      currentOffset += relocation.emit(streamer);
    }

    // If we have emitted the last relocation, but there is still space before
    // the specified end point. This simply means we have raw data at the end.
    if (currentOffset < targetEndOffset) {
      streamer->emitBytes(
          FunctionContents.slice(currentOffset, targetEndOffset));
      currentOffset = targetEndOffset;
    }

    return currentOffset;
  };

  streamer->emitCodeAlignment(
      llvm::Align(function.getConstantIslandAlignment()), &*binaryContext->STI);

  // We are now going to be emitting the original constant islands, that are
  // possibly spread out, into one big constant island at the end of the
  // function. This label denotes the start of this island.
  streamer->emitLabel(function.getFunctionConstantIslandLabel());

  // Sequentially emit each island chunk one after the other.
  for (const IslandChunk &chunk : islandChunks) {
    uint64_t CurrentOffset = chunk.startOffset;

    auto relocationsStart = chunk.relocations.begin();
    const auto relocationsEnd = chunk.relocations.end();

    // Emit the data and relocations between the currentOffset and the next
    // jump label. Note that each sequence of data needs to start with a
    // label. The first sequnece uses the constant island label as its start
    // symbl. We then emit the label for the next sequence at the end of each
    // loop.
    for (const auto &[endOffset, symbol] : chunk.jumpLabels) {

      // TODO this should ideally be constructed in place in the
      // BuildIslandChunks. That way, we can place the constant island label
      // as the label in the first sequence instead of adding it at the end
      // like this. We would also be able to just loop everything nicely here.
      //
      // Find the relocations to emit from the range. We are
      // essentially sliding a window across the range to account for the
      // offset we are trying to emit.
      const auto relocationSubRangeEnd =
          std::ranges::lower_bound(relocationsStart, relocationsEnd, endOffset,
                                   {}, &llvm::bolt::Relocation::Offset);
      const auto relocations =
          std::ranges::subrange(relocationsStart, relocationSubRangeEnd);

      CurrentOffset = emitDataInRange(CurrentOffset, endOffset, relocations);

      // In case this symbol has already been emitted else where. Emitting the
      // same symbol twice leads to a crash.
      if (symbol->isUndefined()) {
        streamer->emitLabel(symbol);
      }

      // Update the start of the next range to be the end of the current
      // subrange of relocations we just emitted.
      relocationsStart = relocationSubRangeEnd;
    }

    // Emit the last data sequence.
    CurrentOffset = emitDataInRange(
        CurrentOffset, chunk.endOffset,
        std::ranges::subrange(relocationsStart, relocationsEnd));
  }
}

void EmitFunctionBody(llvm::bolt::BinaryContext *binaryContext,
                      llvm::bolt::BinaryFunction &function,
                      llvm::bolt::FunctionFragment &functionFragment,
                      llvm::MCStreamer *streamer) {

  for (auto *const basicBLock : functionFragment) {

    // Emit start label and possible secondary entry point symbols.
    streamer->emitLabel(basicBLock->getLabel());
    if (auto *EntrySymbol =
            function.getSecondaryEntryPointSymbol(*basicBLock)) {
      streamer->emitLabel(EntrySymbol);
    }

    for (auto &instruciton : *basicBLock) {

      // If there is such a label, then it means that it needs to be emitted
      // before the instruction to mark its location
      if (auto *instrLabel = binaryContext->MIB->getInstLabel(instruciton)) {
        streamer->emitLabel(instrLabel);
      }

      // Make sure to emit NOPs of correct size. This is a workaround for
      // invalid handling of NOPs in llvm.
      const auto size = binaryContext->MIB->getSize(instruciton);
      if (binaryContext->MIB->isNoop(instruciton) && size.has_value()) {
        llvm::SmallString<15> code;
        llvm::raw_svector_ostream VecOS(code);
        binaryContext->MAB->writeNopData(VecOS, *size,
                                         binaryContext->STI.get());
        streamer->emitBytes(code);
        continue;
      }

      // Remove BOLT specific annotations if there are any, and then emit.
      binaryContext->MIB->stripAnnotations(instruciton);
      streamer->emitInstruction(instruciton, *binaryContext->STI);
    }
  }

  // If the function contains function islands, gather them all up and emit
  // them at the end of the function as one contiguous constant island.
  if (function.hasIslandsInfo()) {
    emitConstantIsland(binaryContext, function, streamer);
  }
}

bool skipFunction(llvm::bolt::BinaryFunction &function) {
  return function.isPseudo() || function.isIgnored() || !function.isSimple() ||
         function.size() == 0 ||
         function.getState() == llvm::bolt::BinaryFunction::State::Empty;
}

void emitFunctions(llvm::bolt::BinaryContext *binaryContext,
                   llvm::MCStreamer *streamer) {
  for (auto &[_, function] : binaryContext->getBinaryFunctions()) {

    if (skipFunction(function)) {
      continue;
    }

    auto &functionFragement = function.getLayout().getMainFragment();
    const auto fragmentNumber = functionFragement.getFragmentNum();

    auto section = binaryContext->getCodeSection(
        function.getCodeSectionName(fragmentNumber));
    section->setHasInstructions(true);

    streamer->switchSection(section);
    streamer->emitCodeAlignment(function.getAlign(), binaryContext->STI.get());

    // Emit entry function labels
    for (auto *functionEntryAlias : function.getSymbols()) {

      forwardSymbolAttributes(
          functionEntryAlias,
          binaryContext->getBinaryDataByName(functionEntryAlias->getName()),
          streamer);

      streamer->emitSymbolAttribute(functionEntryAlias,
                                    llvm::MCSA_ELF_TypeFunction);
      streamer->emitLabel(functionEntryAlias);
    }

    EmitFunctionBody(binaryContext, function, functionFragement, streamer);

    auto *endSymbol = function.getFunctionEndLabel(fragmentNumber);
    streamer->emitLabel(endSymbol);

    // Set the size of the function so that it is visible in the symbol table.
    auto *const startSymbol = function.getSymbol(fragmentNumber);
    auto &context = streamer->getContext();
    const auto *sizeExpr = llvm::MCBinaryExpr::createSub(
        llvm::MCSymbolRefExpr::create(endSymbol, context),
        llvm::MCSymbolRefExpr::create(startSymbol, context), context);
    streamer->emitELFSize(startSymbol, sizeExpr);
  }
}

/// TODO, split the emit code into sub folders when done. For example:
/// Emit/Functions/EmitFunctions
///                EmitConstantIslands
///     /Data     /...
Error EmitObjectFile(llvm::bolt::BinaryContext *binaryContext,
                     const std::string &outputPath) {

  // Open the output file stream
  std::error_code errorCode;
  llvm::ToolOutputFile outFile(outputPath, errorCode, llvm::sys::fs::OF_None);

  RET_ON_TRUE(errorCode, "Cannot open output file: '" + outputPath +
                             "' with error '" + errorCode.message() + "'");

  // Create and initialize the streamer.
  std::unique_ptr<llvm::MCStreamer> streamer =
      binaryContext->createStreamer(outFile.os());
  streamer->initSections(false, *binaryContext->STI);
  streamer->setUseAssemblerInfoForParsing(false);

  // Emit text and data sections.
  emitFunctions(binaryContext, streamer.get());
  emitDataSections(binaryContext, streamer.get());

  streamer->finish();

  RET_ON_TRUE(streamer->getContext().hadError(),
              "LLVM MCStreamer encountered an error during emission.");

  outFile.keep();
  streamer->setUseAssemblerInfoForParsing(true);

  return ERU_SUCCESS;
}

} // namespace Rewriter