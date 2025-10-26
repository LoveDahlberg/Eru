

// llvm
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>

#include <Frontend/Actions.h>
#include <Frontend/Compiler.h>
#include <Frontend/Linker.h>
#include <Support/Log.h>
#include <Support/Triple.h>

namespace Driver {

// Only show command line args marked with this category, prevents displaying of
// llvm generic args.
static llvm::cl::OptionCategory compilerCategory("Compiler options");

llvm::cl::opt<std::string>
    OutputFilename("o", llvm::cl::desc("Specify output filename."),
                   llvm::cl::value_desc("filename"),
                   llvm::cl::cat(compilerCategory), llvm::cl::Required);

llvm::cl::opt<std::string> InputFilename(llvm::cl::Positional,
                                         llvm::cl::desc("<input file>"),
                                         llvm::cl::init("-"),
                                         llvm::cl::Required);

llvm::cl::opt<bool>
    CompileAndDontLink("c",
                       llvm::cl::desc("Only compile to object file, dont link"),
                       llvm::cl::cat(compilerCategory));

llvm::cl::opt<std::string>
    TargetTriple("target", llvm::cl::desc("Specify target triple."),
                 llvm::cl::cat(compilerCategory));

} // namespace Driver

// - Handles parsing of arguments
// - It also calls getAction based on the arguments.
// - Then it calls compiler with the action on each source file.
// - The compiler will compile and perform the action.
// - Then, depending on the commandline arguments, it will call the
//   Linker.cpp.
int main(int argc, char *argv[]) {

  // Setup llvm commandline lib.
  llvm::cl::SetVersionPrinter(
      [](llvm::raw_ostream &OS) { OS << "Compiler 0.1.0\n"; });
  llvm::cl::HideUnrelatedOptions(Driver::compilerCategory);
  llvm::cl::ParseCommandLineOptions(argc, argv);

  // Retrive the command line args.
  const std::filesystem::path &inputFile = std::string(Driver::InputFilename);
  const std::filesystem::path &outputFile = std::string(Driver::OutputFilename);
  const bool compileOnly = Driver::CompileAndDontLink;

  auto maybeTarget = Support::GetSupportedTriple(Driver::TargetTriple);
  ExitAndPrintOnError(maybeTarget);
  const std::string &target = *maybeTarget;

  // TODO create a temporary file in better way.
  auto objectFileOutput = compileOnly ? outputFile : "/tmp/tmp";

  // Determine action to use. For now just use EmitObjectFile action.
  auto *action = new Frontend::Action::EmitObjectFile(objectFileOutput, target);

  // Compiling.
  // For now, just run it on a single source file.
  ExitAndPrintOnError(Frontend::Compiler::Compile(action, inputFile));

  // Linking.
  if (!compileOnly) {
    // Call linker and pass the filepaths to:
    // - Passed object files (start by just supporting single source file)
    // - The created object files
    ExitAndPrintOnError(Frontend::Linker::Link(
        {objectFileOutput},
        {.outputPath = outputFile, .executablePath = argv[0]}));
  }

  return 0;
}