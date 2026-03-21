#include <Frontend/BinaryRewriter.h>
#include <Rewriter/Rewriter.h>
#include <Support/Result.h>

// llvm
#include <llvm/Object/ELFObjectFile.h>
#include <llvm/Object/ObjectFile.h>
#include <llvm/Support/TargetSelect.h>

// bolt
#include <bolt/Rewrite/RewriteInstance.h>
#include <bolt/Passes/BinaryPasses.h>

namespace Frontend::Action {

Error BinaryRewriter::ActOn(const Support::IO::Files &files) {

  // Initialize LLVM targets — required before anything BOLT does
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllDisassemblers();

  auto binary = files.GetObjectFiles().at(0).string();

  // Open the object file
  auto objOrErr = llvm::object::ObjectFile::createObjectFile(binary);

  auto *elfObj =
      llvm::dyn_cast<llvm::object::ELFObjectFileBase>(objOrErr->getBinary());

  auto maybeRewriteInstance =
      llvm::bolt::RewriteInstance::create(elfObj, 0, nullptr, "eru");

      // ...
  return ERU_SUCCESS;
}

} // namespace Frontend::Action

namespace Frontend {

Error Rewrite(Action::BinaryAction *action, const Support::IO::Files &file) {

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