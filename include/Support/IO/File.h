#pragma once

#include <filesystem>
#include <tuple>
#include <vector>

#include <Support/Environment.h>
#include <Support/Result.h>

namespace Support::IO {

constexpr auto tmpDirectoryName = ".eru";
std::filesystem::path createTmpDir();

using ObjectArdaInvalidFiles = std::tuple<std::vector<std::filesystem::path>,
                                          std::vector<std::filesystem::path>,
                                          std::vector<std::filesystem::path>>;

ObjectArdaInvalidFiles
SplitInputFiles(const std::vector<std::filesystem::path> &parsedInputFiles);

Result<std::string> getFileContent(const std::filesystem::path &path);

class Files {
public:
  // Default constructor for Result.h
  Files() : compileOnly(false) {}

  Files(const std::vector<std::filesystem::path> compilableInputFiles,
        const std::vector<std::filesystem::path> objectFiles,
        const std::filesystem::path finalOutputPath, const bool compileOnly)
      : directory(createTmpDir()), compilableInputFiles(compilableInputFiles),
        finalOutputPath(finalOutputPath), objectFiles(objectFiles),
        compileOnly(compileOnly) {}

  const std::vector<std::filesystem::path> &getcompilableInputFiles() {
    return compilableInputFiles;
  }

  void AddObjectFile(std::filesystem::path objectFile) {
    objectFiles.push_back(objectFile);
  }

  const std::vector<std::filesystem::path> &GetObjectFiles() {
    return objectFiles;
  }

  bool isCompileOnly() { return compileOnly; }

  bool isSingleArda() { return compilableInputFiles.size() == 1; }

  const std::filesystem::path &getFinalOutputPath() { return finalOutputPath; }

  const std::filesystem::path
  CreateFileTmpPath(std::filesystem::path originalPath);

private:
  std::filesystem::path directory;
  const std::vector<std::filesystem::path> compilableInputFiles;
  const std::filesystem::path finalOutputPath;
  std::vector<std::filesystem::path> objectFiles;
  const bool compileOnly;

  std::vector<std::filesystem::path> binaryFiles;
};

} // namespace Support::IO