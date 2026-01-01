

#include <filesystem>

// llvm
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>

#include <Frontend/Actions.h>
#include <Frontend/Compiler.h>
#include <Frontend/Linker.h>
#include <Support/Commandline.h>
#include <Support/IO/File.h>
#include <Support/Log.h>
#include <Support/Triple.h>

namespace Driver {

// Only show command line args marked with this category, prevents displaying of
// llvm generic args.
static llvm::cl::OptionCategory compilerCategory("Compiler options");

llvm::cl::opt<std::string>
    OutputFilename("o", llvm::cl::desc("Specify output filename."),
                   llvm::cl::value_desc("filename"),
                   llvm::cl::cat(compilerCategory));

llvm::cl::list<std::string> InputFilename(llvm::cl::Positional,
                                          llvm::cl::OneOrMore,
                                          llvm::cl::desc("<input files>"));

llvm::cl::opt<bool>
    CompileAndDontLink("c",
                       llvm::cl::desc("Only compile to object file, dont link"),
                       llvm::cl::cat(compilerCategory));

llvm::cl::opt<bool> EmitLLVM(
    "emit-llvm",
    llvm::cl::desc(
        "Emit llvm IR in current working directory or in ERU_TMP_PATH if set."),
    llvm::cl::cat(compilerCategory));

llvm::cl::opt<std::string>
    TargetTriple("target", llvm::cl::desc("Specify target triple."),
                 llvm::cl::cat(compilerCategory));

} // namespace Driver

int main(int argc, char *argv[]) {

  // Setup llvm commandline lib.
  llvm::cl::SetVersionPrinter(
      [](llvm::raw_ostream &OS) { OS << "Compiler 0.1.0\n"; });
  llvm::cl::HideUnrelatedOptions(Driver::compilerCategory);
  llvm::cl::ParseCommandLineOptions(argc, argv);

  // Retrive the command line args.
  const auto inputFiles = []() {
    std::vector<std::filesystem::path> paths;
    for (auto file : Driver::InputFilename) {
      paths.push_back(file);
    }
    return paths;
  }();

  const auto &outputFile = Driver::OutputFilename.getValue();
  const bool compileOnly = Driver::CompileAndDontLink;
  const bool emitLLVM = Driver::EmitLLVM;

  // Verify that the commandline arguments are correct and setup temporary
  // directory and file paths.
  auto maybefiles = Support::VerifyCommandLineAndCreateFiles(
      inputFiles, outputFile, compileOnly);
  Support::ExitAndPrintOnError(maybefiles);
  auto files = *maybefiles;

  // Get a supported target triple to use.
  auto maybeTarget = Support::GetSupportedTriple(Driver::TargetTriple);
  Support::ExitAndPrintOnError(maybeTarget);
  const std::string &target = *maybeTarget;

  // Determine action to use. For now just use EmitObjectFile action.
  auto *action = new Frontend::Action::EmitObjectFile(files, target, emitLLVM);

  // Compiling.
  Support::ExitAndPrintOnError(Frontend::Compiler::Compile(action, files));

  // Linking.
  if (!compileOnly) {
    Support::ExitAndPrintOnError(
        Frontend::Linker::Link(files, {.executablePath = argv[0]}));
  }

  return 0;
}