#include <Frontend/Linker.h>
#include <Support/Environment.h>
#include <Support/Result.h>

#include <filesystem>
#include <llvm/Support/FileSystem.h>

namespace Frontend::Linker {

namespace {
Result<std::string> FindAinur(const std::filesystem::path &executablePath) {

  // TODO add more methods of looking, system install locations, in link include
  // directories with -L (if added of course).

  // Check environment variable override first
  if (auto envPath = std::getenv(ainurPath)) {
    return std::string(envPath);
  }

  // TODO: This is a development path.
  auto path =
      executablePath.parent_path().parent_path() / "Ainur" / "libAinur.a";

  RET_ON_FALSE(std::filesystem::exists(path),
               "Ainur not in '" + path.string() + "'");

  return path.string();
}

} // namespace

Error Link(Support::IO::Files &files, const LinkerData data) {

  auto maybeAinur = FindAinur(data.executablePath);
  RET_ON_FAILURE(maybeAinur, "Link: Cannot find path to Ainur.");

  auto AinurPath = *maybeAinur;

  // TODO: Discover the linker path.
  std::string linkerCmd = "ld.lld " + AinurPath;

  for (const auto &objectFile : files.GetObjectFiles()) {
    linkerCmd += " " + objectFile.string();
  }

  linkerCmd += " -o " + files.getFinalOutputPath().string();

  RET_ON_FALSE(std::system(linkerCmd.c_str()) == 0,
               "Link: Failed to link with invocation:\n\n" + linkerCmd);

  return SUCCESSFUL;
}

} // namespace Frontend::Linker