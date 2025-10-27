#include <filesystem>
#include <fstream>

#include <Support/IO/File.h>

#include <llvm/Support/raw_ostream.h>
#include <tuple>

namespace Support::IO {

std::filesystem::path createTmpDir() {

  std::filesystem::path folder = []() {

    // Check environment path override.
    if (auto envPath = std::getenv(tmpDirectoryPath)) {
      return std::filesystem::path(envPath);
    }
    // Base it on cwd.
    return std::filesystem::current_path();
  }() / tmpDirectoryName;

  std::filesystem::create_directory(folder);
  return folder;
}

Result<std::string> getFileContent(const std::filesystem::path &path) {
  std::string content;
  std::ifstream file(path);
  RET_ON_FALSE(file.is_open(),
               "Unable to open file " + path.filename().string());
  content.assign((std::istreambuf_iterator<char>(file)),
                 std::istreambuf_iterator<char>());
  file.close();
  return content;
}

ObjectArdaInvalidFiles
SplitInputFiles(const std::vector<std::filesystem::path> &parsedInputFiles) {
  std::vector<std::filesystem::path> ObjectFiles;
  std::vector<std::filesystem::path> ArdaFiles;
  std::vector<std::filesystem::path> InvalidFiles;

  for (const auto &file : parsedInputFiles) {
    if (file.extension() == ".o") {
      ObjectFiles.push_back(file);
      continue;
    }

    if (file.extension() == ".arda") {
      ArdaFiles.push_back(file);
      continue;
    }

    InvalidFiles.push_back(file);
  }

  return std::make_tuple(ObjectFiles, ArdaFiles, InvalidFiles);
}

const std::filesystem::path
Files::CreateFileTmpPath(std::filesystem::path originalPath) {

  if (originalPath.extension() == ".arda") {
    originalPath.replace_extension(".o");
  }
  return directory / originalPath.filename();
}

} // namespace Support::IO