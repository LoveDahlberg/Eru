

#include <Support/Result.h>

#include <initializer_list>
#include <string>

// llvm
#include <llvm/TargetParser/Host.h>
#include <llvm/TargetParser/Triple.h>

namespace Support {

constexpr auto supportedArchs = {"x86_64"};
constexpr auto supportedTargets = {"x86_64-unknown-linux-gnu",
                                   "x86_64-pc-linux-gnu"};

/// Get the triple from the target argument passed by the user.
Result<llvm::Triple> GetSupportedTriple(std::string &target);

} // namespace Support