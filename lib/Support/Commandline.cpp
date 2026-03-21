
#include "Support/Result.h"
#include <Support/Commandline.h>
#include <Support/Log.h>

#include <filesystem>

namespace Support {

Result<IO::Files> VerifyRewriterCommandLineAndCreateFiles(
    const std::filesystem::path &parsedInputFile,
    const std::filesystem::path &parsedOutputFile) {
  RET_ON_TRUE(parsedInputFile.empty(),
              "Must specifiy one input file to rewriter.");

  RET_ON_FALSE(parsedInputFile.extension() == ".o",
               "Must pass object file to rewriter.");

  RET_ON_TRUE(parsedOutputFile.empty(),
              "Must specifiy one output file to rewriter.");

  RET_ON_TRUE(parsedInputFile == parsedOutputFile,
              "Error: Input file same as output file.");

  return IO::Files(parsedInputFile, parsedOutputFile);
}

Result<IO::Files> VerifyEruCommandLineAndCreateFiles(
    const std::vector<std::filesystem::path> &parsedInputFiles,
    const std::filesystem::path &parsedOutputFile, const bool compileOnly) {

  RET_ON_TRUE(parsedInputFiles.empty(),
              "Must specifiy at least one input file");

  auto [ObjectFiles, ArdaFiles, Invalid] =
      IO::SplitInputFiles(parsedInputFiles);

  RET_ON_FALSE(
      Invalid.empty(),
      "Error: Found input file(s) of unknown type " +
          Support::vectorToPrintableString(Invalid,
                                           [](std::filesystem::path &path) {
                                             return path.filename().string();
                                           }) +
          ". Only .arda and .o files are supported as input.");

  // Cases
  // -compile only
  //  - One input, no output OK
  //    - Create output next to input with .o
  //    - Stop if input is object file.
  //  - One input, one output OK
  //     - Create the output like they expect.
  //     - Error if input is same as output.
  //     - Stop if input is object file
  //  - Multiple input, no output OK
  //    - Create output next to the first input with .o.
  //    - Error if output is going to result in the same as any input.
  //    - Stop if all input are object files.
  //  - Multiple input, one output ERROR
  //
  // - Compile and link
  //  - One input, no output OK
  //   - Create output next to input without file extension
  //   - Error if output is going to result in the same as input.
  //  - One input, one output OK
  //     - Create the output like they expect.
  //     - Error if input is same as output.
  //  - Multiple input, no output OK
  //    - Create output next to the first input with .o.
  //    - Error if output is going to result in the same as any input.
  //  - Multiple input, one output OK
  //     - Create the output like they expect.

  if (compileOnly) {
    // Error out if we have only object files as input in compile-only mode.
    // This would not do anything, as we do not touch it during compilation.
    RET_ON_TRUE(!ObjectFiles.empty() && ArdaFiles.empty(),
                "Nothing to do. Single object file " +
                    ObjectFiles.at(0).filename().string() +
                    " provided in compile-only mode.");

    // Error out if output files are specified in compile only mode with
    // multiple input files.
    RET_ON_TRUE(!parsedOutputFile.empty() && parsedInputFiles.size() > 1,
                "Error: cannot specify output file with multiple input files "
                "when in compile-only mode.");
  }

  // Extension to use for the final output file.
  const auto extension = compileOnly ? ".o" : "";

  // If multiple input files, pick the first one to base the output file name
  // on; in case one is not specified. Note that in case of compileOnly, this
  // will not be used.
  const auto &chosenInputFile = parsedInputFiles.at(0);

  // The final output file. Either the user specified one, or generated based on
  // mode and the first input file.
  const auto finalOutputFile =
      parsedOutputFile.empty()
          ? std::filesystem::path(chosenInputFile).replace_extension(extension)
          : parsedOutputFile;

  // Error out if the output file would result in the same as the input
  // file.
  RET_ON_TRUE(finalOutputFile == chosenInputFile,
              "Error: Input file same as output file.");

  // Compile only:
  //  Input(s) are:
  //   - <name-0>.arda .. <name-n>.arda
  //  Output is:
  //   - <name-0>.o or <user-provided-file>.
  //
  // Compile and Link:
  //  Input(s) are:
  //   - <name-0>.arda .. <name-n>.arda
  //     MIXED WITH
  //   - <name-0>.o .. <name-n>.o
  //  Output is:
  //  - <name-0> or <user-provided-path> after linking.
  //
  return IO::Files(ArdaFiles, ObjectFiles, finalOutputFile, compileOnly);
}

} // namespace Support