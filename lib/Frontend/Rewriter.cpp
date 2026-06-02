

#include "llvm/Object/Binary.h"
#include <Frontend/Actions.h>
#include <Rewriter/Disassemble.h>
#include <Rewriter/Discover/Discover.h>
#include <Rewriter/Emit.h>

// llvm
#include <llvm/Support/TargetSelect.h>

namespace Frontend {

namespace {

Result<llvm::bolt::BinaryContext *>
Initialize(const std::string &objectFilePath, llvm::ObjectFile *objectFile) {

  // Initialize LLVM targets — required before anything BOLT does
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllDisassemblers();

  // 1. Initialize
  llvm::ELF64LEObjectFile *ELF64LEFile =
      llvm::dyn_cast<llvm::ELF64LEObjectFile>(objectFile);
  assert(ELF64LEFile != nullptr && "object file not ELF 64 bit little endian");

  llvm::bolt::Relocation::Arch = ELF64LEFile->makeTriple().getArch();

  // Create binary context.
  auto maybeBC = llvm::bolt::BinaryContext::createBinaryContext(
      ELF64LEFile->makeTriple(),
      std::make_shared<llvm::orc::SymbolStringPool>(), objectFilePath, nullptr,
      true,
      llvm::DWARFContext::create(
          *ELF64LEFile, llvm::DWARFContext::ProcessDebugRelocations::Ignore,
          nullptr, "", llvm::WithColor::defaultErrorHandler,
          llvm::WithColor::defaultWarningHandler),
      llvm::bolt::JournalingStreams{llvm::outs(), llvm::errs()});
  if (auto err = maybeBC.takeError()) {
    return {"Error creating binary context."};
  }
  auto binaryContext = std::move(maybeBC.get());

  // Add MCPlusBuilder as target builder in context
  binaryContext->initializeTarget(std::unique_ptr<llvm::bolt::MCPlusBuilder>(
      llvm::bolt::createX86MCPlusBuilder(
          binaryContext->MIA.get(), binaryContext->MII.get(),
          binaryContext->MRI.get(), binaryContext->STI.get())));

  // Sanity check that we are processing x86.
  assert(binaryContext->isX86());

  return binaryContext.get();
}
} // namespace

Error RewriteObject(Action::RewriteAction *action,
                    const Support::IO::Files &file) {

  // Open the object file.
  const std::string objectFilePath = file.GetObjectFiles().at(0).string();
  auto objOrErr = llvm::object::ObjectFile::createObjectFile(objectFilePath);
  if (llvm::Error Err = objOrErr.takeError()) {
    return {"Error creating object file."};
  }

  // Has to stay in scope.
  auto &owningBinary = *objOrErr;

  auto maybeContext = Initialize(objectFilePath, owningBinary.getBinary());
  RET_ON_FAILURE(maybeContext, "Failed to initialize rewrite object.");
  auto binaryContext = *maybeContext;

  RET_ON_FAILURE(
      Rewriter::DiscoverAndRegisterObject(
          llvm::dyn_cast<llvm::ELF64LEObjectFile>(owningBinary.getBinary()),
          binaryContext),
      "Failed to discover rewriter object");

  RET_ON_FAILURE(Rewriter::DisassembleAndBuildCFG(binaryContext),
                 "Failed to disassemble and build CFG rewrite object");

  // Run passes
  RET_ON_FAILURE(action->ActOn(file), "Failed actOn rewrite object.");

  RET_ON_FAILURE(
      Rewriter::EmitObjectFile(binaryContext, file.getFinalOutputPath()),
      "Failed to emit object file.");

  return ERU_SUCCESS;
}

} // namespace Frontend