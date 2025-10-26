#include <Support/Triple.h>

namespace Support {

Result<std::string> GetSupportedTriple(std::string &target) {
  // Triple is defined as 'arch-vendor-os-environment'
  // - arch: x86_64, aarch64, arm, riscv64 ..
  // - vendor: pc, apple, unknown ..
  // - os: linux, darwin, windows ..
  // = environment: gnu, musl, android, eabi .. (optional)

  // No target passed, use system default.
  if (target.empty()) {
    target = llvm::sys::getDefaultTargetTriple();
  }

  // Incomplete triple.
  if (target.find('-') == std::string::npos) {

    // Assume that only arch was specified.
    RET_ON_FALSE(std::find(supportedArchs.begin(), supportedArchs.end(),
                           target) != supportedArchs.end(),
                 "GetSupportedTriple: specified target '" + target +
                     "' is not supported.");

    // Guess the triple, use the first supported matchin target that appears in
    // supportedTargets.
    auto triple = std::find_if(
        supportedTargets.begin(), supportedTargets.end(),
        [&target](auto item) { return std::string(item).starts_with(target); });

    RET_ON_TRUE(triple == supportedTargets.end(),
                "GetSupportedTriple: Could not find a matching supported "
                "architecture for '" +
                    target + "'");
    target = *triple;
  }

  RET_ON_FALSE(std::find(supportedTargets.begin(), supportedTargets.end(),
                         target) != supportedTargets.end(),
               "GetSupportedTriple: Could not find a matching supported "
               "architecture for '" +
                   target + "'");
  return target;
}

} // namespace Support