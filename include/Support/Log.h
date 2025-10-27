

#include <llvm/Support/raw_ostream.h>

namespace Support {

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

template <typename kind, typename stringPredicate>
std::string vectorToPrintableString(std::vector<kind> vector,
                                    stringPredicate &&transform) {
  std::string output = "'";
  std::string prefix;
  std::string suffix;
  for (int i = 0 ; i < vector.size() ; i++) {
    prefix = i == 0 ? "" : " ";
    suffix = i == vector.size() -1 ? "" : ",";
    output += prefix + transform(vector[i]) + suffix;
  }
  output += "'";
  return output;
}

} // namespace Support