

// llvm
#include "Frontend/Compiler.h"
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>

#include <Frontend/Actions.h>

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

} // namespace Driver

// - Handles parsing of arguments
// - It also calls getAction based on the arguments.
// - Then it calls compiler with the action on each source file.
// - The compiler will compile and perform the action.
// - Then, depending on the commandline arguments, it will call the
//   Linker.cpp.
int main(int argc, char *argv[]) {

  llvm::cl::SetVersionPrinter(
      [](llvm::raw_ostream &OS) { OS << "Compiler 0.1.0\n"; });

  llvm::cl::HideUnrelatedOptions(Driver::compilerCategory);

  llvm::cl::ParseCommandLineOptions(argc, argv);

  const std::string &inputFile = Driver::InputFilename;
  const std::string &outputFile = Driver::OutputFilename;
  const bool compileOnly = Driver::CompileAndDontLink;

  // Determine action to use.
  Frontend::Action::Action* action;
  if (compileOnly) {
      action = new Frontend::Action::EmitObjectFile(outputFile);
  } else {
    llvm::outs() << "Only supports compile only with -c.\n";
    exit(1);
  }

  // For now, just run it on a single source file.
  auto result = Frontend::Compiler::Compiler(action, inputFile);

  if(!result.check()){
    // print stack trace.

    llvm::outs()  << "\nFailure:\n";
    if (!result.failureDescription.empty()) {
      llvm::outs()  << result.failureDescription.front() << "\n";
    }
    llvm::outs()  << result.codeSnippet << "\n";

    llvm::outs()  << "\n\n";
  }

  return 0;
}