

#include <Support/IO/File.h>
#include <Support/Result.h>

#include <filesystem>

namespace Support {

Result<IO::Files> VerifyEruCommandLineAndCreateFiles(
    const std::vector<std::filesystem::path> &inputFiles,
    const std::filesystem::path &outputFile, const bool compileOnly);

Result<IO::Files> VerifyRewriterCommandLineAndCreateFiles(
    const std::filesystem::path &parsedInputFile,
    const std::filesystem::path &parsedOutputFile);

} // namespace Support