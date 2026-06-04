

// bolt
#include "llvm/MC/MCSection.h"
#include <bolt/Core/BinaryFunction.h>

// llvm
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/ToolOutputFile.h>

#include <Rewriter/Emit.h>

namespace Rewriter {

void emitDataSections();

void EmitFunctionBody(llvm::bolt::BinaryContext *binaryContext,
                      llvm::bolt::BinaryFunction *function,
                      llvm::bolt::FunctionFragment &functionFragment,
                      llvm::MCStreamer *streamer) {

  for (auto *const basicBLock : functionFragment) {

    // Emit start label and possible secondary entry point symbols.
    streamer->emitLabel(basicBLock->getLabel());
    if (auto *EntrySymbol =
            function->getSecondaryEntryPointSymbol(*basicBLock)) {
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

  // emitConstantIslands
}

bool skipFunction(llvm::bolt::BinaryFunction *function) {
  return function->isPseudo() || function->isIgnored() ||
         !function->isSimple() || function->size() == 0 ||
         function->getState() == llvm::bolt::BinaryFunction::State::Empty;
}

void emitFunctions(llvm::bolt::BinaryContext *binaryContext,
                   llvm::MCStreamer *streamer) {
  for (auto *function : binaryContext->getOutputBinaryFunctions()) {

    if (skipFunction(function)) {
      continue;
    }

    auto &functionFragement = function->getLayout().getMainFragment();
    const auto fragmentNumber = functionFragement.getFragmentNum();

    auto section = binaryContext->getCodeSection(
        function->getCodeSectionName(fragmentNumber));
    section->setHasInstructions(true);

    streamer->switchSection(section);
    streamer->emitCodeAlignment(function->getAlign(), binaryContext->STI.get());

    // Emit entry function labels
    for (auto *functionEntryAlias : function->getSymbols()) {
      streamer->emitSymbolAttribute(functionEntryAlias,
                                    llvm::MCSA_ELF_TypeFunction);
      streamer->emitLabel(functionEntryAlias);
    }

    EmitFunctionBody(binaryContext, function, functionFragement, streamer);

    auto *endSymbol = function->getFunctionEndLabel(fragmentNumber);
    streamer->emitLabel(endSymbol);

    // Set the size of the function so that it is visible in the symbol table.
    auto *const startSymbol = function->getSymbol(fragmentNumber);
    auto &context = streamer->getContext();
    const auto *sizeExpr = llvm::MCBinaryExpr::createSub(
        llvm::MCSymbolRefExpr::create(endSymbol, context),
        llvm::MCSymbolRefExpr::create(startSymbol, context), context);
    streamer->emitELFSize(startSymbol, sizeExpr);
  }
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

  streamer->initSections(false, *binaryContext->STI);
  streamer->setUseAssemblerInfoForParsing(false);

  binaryContext->getTextSection()->setAlignment(
      llvm::Align(binaryContext->PageAlign));

  emitFunctions(binaryContext, streamer.get());

  emitDataSections();

  /// OLD Emitter TODO REMOVE

  // Emit binary functions
  streamer->switchSection(binaryContext->getTextSection());
  for (auto &[_, function] : binaryContext->getBinaryFunctions()) {

    // TODO add flag for this and then write to file instead.
    llvm::outs() << "Dumping function: " << function.getPrintName() << "\n";
    function.print(llvm::outs(), "Before Emit");

    streamer->emitSymbolAttribute(function.getSymbol(), llvm::MCSA_Global);
    streamer->emitLabel(function.getSymbol());

    for (llvm::bolt::BinaryBasicBlock *bb : function.getLayout().blocks()) {

      streamer->emitLabel(bb->getLabel());
      for (llvm::MCInst &inst : *bb) {
        binaryContext->MIB->stripAnnotations(inst);
        streamer->emitInstruction(inst, *binaryContext->STI);
      }
    }
  }

  // Emit data
  for (auto &[address, data] : binaryContext->getBinaryData()) {
    llvm::bolt::BinarySection &section = data->getSection();

    // We only want to emit data here (e.g. .rodata, .data).
    // Code and unallocated sections (like .symtab) are handled elsewhere.
    if (section.isText() || !section.isAllocatable() || section.isVirtual()) {
      continue;
    }

    llvm::MCSection *mcSection = binaryContext->Ctx->getELFSection(
        section.getName(), section.getELFType(), section.getELFFlags());
    streamer->switchSection(mcSection);

    streamer->emitValueToAlignment(llvm::Align(data->getAlignment()));

    streamer->emitSymbolAttribute(data->getSymbol(), llvm::MCSA_Local);
    streamer->emitLabel(data->getSymbol());

    uint64_t offsetInSection = data->getAddress() - section.getAddress();
    llvm::StringRef dataBytes =
        section.getContents().substr(offsetInSection, data->getSize());

    streamer->emitBytes(dataBytes);
  }

  streamer->finish();

  RET_ON_TRUE(streamer->getContext().hadError(),
              "LLVM MCStreamer encountered an error during emission.");

  outFile.keep();

  return ERU_SUCCESS;
}

} // namespace Rewriter