#include <Frontend/Actions.h>
#include <IR/IRGenerator.h>
#include <Support/Result.h>

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

Error emitObject(llvm::Module &module, const std::filesystem::path &outputFile,
                 const std::string &targetTriple) {

  // From kolidascope. TODO rewrite this with the new pass manager.

  // Initialize targets for emitting object code.
  llvm::InitializeAllTargetInfos();
  llvm::InitializeAllTargets();
  llvm::InitializeAllTargetMCs();
  llvm::InitializeAllAsmParsers();
  llvm::InitializeAllAsmPrinters();

  std::string Error;
  auto Target = llvm::TargetRegistry::lookupTarget(targetTriple, Error);

  // Print an error and exit if we couldn't find the requested target.
  // This generally occurs if we've forgotten to initialise the
  // TargetRegistry or we have a bogus target triple.
  if (Target == nullptr) {
    return FAIL(
        "EmitObjectFile: Cannot find the reqested target with error '" + Error +
        "'");
  }

  // Target machine, use generic configuration for basic case
  auto CPU = "generic";
  auto Features = "";
  llvm::TargetOptions opt;
  auto TargetMachine = Target->createTargetMachine(targetTriple, CPU, Features,
                                                   opt, llvm::Reloc::PIC_);

  // Set layout and triple in module, may be useful for some passes.
  module.setDataLayout(TargetMachine->createDataLayout());
  module.setTargetTriple(targetTriple);

  // Define some output file
  std::error_code EC;
  llvm::raw_fd_ostream dest(outputFile.string(), EC, llvm::sys::fs::OF_None);

  if (EC) {
    return FAIL("EmitObjectFile: Failed to define output file '" +
                   EC.message() + "'");
  }

  llvm::legacy::PassManager pass;
  auto FileType = llvm::CodeGenFileType::ObjectFile;

  if (TargetMachine->addPassesToEmitFile(pass, dest, nullptr, FileType)) {
    return FAIL("EmitObjectFile: Failed to add pass to emit file.");
  }

  pass.run(module);
  dest.flush();

  return SUCCESSFUL;
}

void writeIRToFile(llvm::Module &module, std::string &errorMsg) {

  auto path = [&]() {
    if (auto envPath = std::getenv(tmpDirectoryPath)) {
      return std::filesystem::path(envPath);
    }
    return std::filesystem::current_path();
  }() / "IR";

  std::error_code EC;
  llvm::raw_fd_ostream out(path.string(), EC, llvm::sys::fs::OF_Text);
  if (EC) {
    errorMsg += " and failed to define output file '" + EC.message() + "'.";
    return;
  }

  module.print(out, nullptr);
  out.flush();

  llvm::outs() << "\nWrote IR to '" << path.string() << "'.";
}

} // namespace

Error EmitObjectFile::ActOn(AST::Context::ASTContext context) {

  auto ctx = new llvm::LLVMContext();
  auto module = llvm::Module("default-module", *ctx);

  auto generator = IR::IRGenerator(module);

  // TODO verify that that there is no error here.

  for (auto result : generator.Walk(context)) {
    if (result.hasFailed) {
      std::string errorMsg = "EmitObjectFile: ActOn: failed to walk AST";
      writeIRToFile(module, errorMsg);

      return FAIL(errorMsg);
    }
  }

  // Cases
  //  Compile only
  //   - Single .arda -> use files.finalOutputPath
  //   - Multiple .arda -> use <name>.o
  //  Compile and link
  //   - Single and multiple .arda -> use .eru/<name>.o

  std::filesystem::path outputFile = [&]() {
    if (files.isCompileOnly()) {
      if (files.isSingleArda()) {
        return files.getFinalOutputPath();
      }
      return context.inputFile.replace_extension(".o");
    }
    return files.CreateFileTmpPath(context.inputFile);
  }();

  // For now just verify the module.
  auto verification = llvm::verifyModule(module, &llvm::errs());
  if (verification || emitLLVM) {
    std::string errorMsg = "EmitObjectFile: Failed to verify module";
    writeIRToFile(module, errorMsg);

    RET_ON_TRUE(verification, errorMsg);
  }

  RET_ON_FAILURE(emitObject(module, outputFile, targetTriple),
                 "EmitObjectFile: Failed to emit object file");

  files.AddObjectFile(outputFile);

  return SUCCESSFUL;
}

} // namespace Frontend::Action