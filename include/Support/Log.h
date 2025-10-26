

#include <llvm/Support/raw_ostream.h>

inline void ExitAndPrintOnError(auto result) {
  // If error occoured
  if (!result.check()) {

    // Print stack trace.
    llvm::outs() << "\nFailure:\n";
    if (!result.failureDescription.empty()) {
      llvm::outs() << result.failureDescription.front() << "\n";
    }
    llvm::outs() << result.codeSnippet << "\n";

    llvm::outs() << "\n\n";
    exit(1);
  }
};

