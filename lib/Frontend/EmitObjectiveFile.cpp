#include <Frontend/Actions.h>
#include <IR/IRGenerator.h>

// llvm
#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/LegacyPassManager.h>
#include <llvm/IR/Module.h>
#include <llvm/IR/Verifier.h>
#include <llvm/MC/TargetRegistry.h>
#include <llvm/Support/FileSystem.h>
#include <llvm/Support/TargetSelect.h>
#include <llvm/Target/TargetMachine.h>
#include <llvm/Target/TargetOptions.h>
#include <llvm/TargetParser/Host.h>

namespace Frontend::Action {

namespace {

bool emitObject(llvm::Module &module, llvm::StringRef filename) {

  // From kolidascope. TODO rewrite this with the new pass manager.

  // Get the default system target triple.
  auto TargetTriple = llvm::sys::getDefaultTargetTriple();

  // Initialize targets for emitting object code.
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  std::string Error;
  auto Target = llvm::TargetRegistry::lookupTarget(TargetTriple, Error);

  // Print an error and exit if we couldn't find the requested target.
  // This generally occurs if we've forgotten to initialise the
  // TargetRegistry or we have a bogus target triple.
  if (!Target) {
    return false;
  }

  // Target machine, use generic configuration for basic case
  auto CPU = "generic";
  auto Features = "";
  llvm::TargetOptions opt;
  auto TargetMachine = Target->createTargetMachine(TargetTriple, CPU, Features,
                                                   opt, llvm::Reloc::PIC_);

  // Set layout and triple in module, may be useful for some passes.
  module.setDataLayout(TargetMachine->createDataLayout());
  module.setTargetTriple(TargetTriple);

  // Define some output file
  auto Filename = "output.o";
  std::error_code EC;
  llvm::raw_fd_ostream dest(Filename, EC, llvm::sys::fs::OF_None);

  if (EC) {
    return false;
  }

  llvm::legacy::PassManager pass;
  auto FileType = llvm::CodeGenFileType::ObjectFile;

  if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
    return false;
  }

  pass.run(module);
  dest.flush();

  return true;
}

} // namespace

// TODO move this to its own file
bool EmitObjectiveFile::ActOn(AST::Context::ASTContext context) {

  constexpr auto &name = "default-module";

  auto ctx = new llvm::LLVMContext();
  auto module = llvm::Module(name, *ctx);

  auto generator = IR::IRGenerator(module);

  // TODO verify that that there is no error here.
  generator.Walk(context);

  // For now just verify the module.
  if (!llvm::verifyModule(module, &llvm::errs())) {
    return false;
  }

  if (!emitObject(module, name)) {
    return false;
  }

  return true;
}

} // namespace Frontend::Action