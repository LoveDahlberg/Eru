
#include <filesystem>

#include <Frontend/Actions.h>
#include <Frontend/BinaryRewriter.h>
#include <Support/Commandline.h>
#include <Support/Log.h>

// llvm
#include <llvm/Support/CommandLine.h>
#include <llvm/Support/raw_ostream.h>

#include <bolt/Utils/CommandLineOpts.h>

namespace Driver::Rewriter {

static llvm::cl::OptionCategory rewriterCategory("Compiler options");

llvm::cl::opt<std::string> InputFilename(llvm::cl::Positional,
                                         llvm::cl::ValueRequired,
                                         llvm::cl::desc("<input file>"));

} // namespace Driver::Rewriter

int main(int argc, char *argv[]) {

  // Setup llvm commandline lib.
  llvm::cl::SetVersionPrinter(
      [](llvm::raw_ostream &OS) { OS << "Rewriter 0.1.0\n"; });
  llvm::cl::HideUnrelatedOptions(Driver::Rewriter::rewriterCategory);

  llvm::cl::ParseCommandLineOptions(argc, argv);

  const auto &inputFile = Driver::Rewriter::InputFilename.getValue();

  // Bolt already registered a OutputFilename, though it is optional.
  const auto &outputFile = opts::OutputFilename.getValue();

  auto maybefiles =
      Support::VerifyRewriterCommandLineAndCreateFiles(inputFile, outputFile);
  Support::ExitAndPrintOnError(maybefiles);
  auto files = *maybefiles;

  // Create an Rewrite action
  auto *action = new Frontend::Action::BinaryRewriter();

  // Call Rewrite on that action
  Support::ExitAndPrintOnError(Frontend::Rewrite(action, files));

  return 0;
}
