

// bolt
#include "llvm/MC/MCSection.h"
#include <bolt/Core/BinaryFunction.h>

// llvm
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/ToolOutputFile.h>

#include <Rewriter/Emit.h>

namespace Rewriter {

void emitDataSections();

void EmitFunctionBody() {}

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

    // All parts of the function to emit. Split fragments might not exist here
    // though.
    auto functionFragments = [&] {
      auto &layout = function->getLayout();

      std::vector<llvm::bolt::FunctionFragment *> mainFragment = {
          &layout.getMainFragment()};
      if (function->isSplit()) {
        for (auto &fragment : layout.getSplitFragments()) {
          mainFragment.push_back(&fragment);
        }
      }
      return mainFragment;
    }();

    for (auto functionFragment : functionFragments) {
      const auto fragmentNumber = functionFragment->getFragmentNum();

      auto section = binaryContext->getCodeSection(
          function->getCodeSectionName(fragmentNumber));
      section->setHasInstructions(true);

      streamer->switchSection(section);
      streamer->emitCodeAlignment(function->getAlign(),
                                  binaryContext->STI.get());

      // Emit entry function labels
      for (auto *functionEntryAlias : function->getSymbols()) {
        streamer->emitSymbolAttribute(functionEntryAlias,
                                      llvm::MCSA_ELF_TypeFunction);
        streamer->emitLabel(functionEntryAlias);
      }

      // Emit body
      EmitFunctionBody();

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

  /// OLD Emitter

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