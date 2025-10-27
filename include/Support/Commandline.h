

#include <Support/IO/File.h>
#include <Support/Result.h>

#include <filesystem>

namespace Support {

Result<IO::Files> VerifyCommandLineAndCreateFiles(
    const std::vector<std::filesystem::path> &inputFiles,
    const std::filesystem::path &outputFile, const bool compileOnly);
}