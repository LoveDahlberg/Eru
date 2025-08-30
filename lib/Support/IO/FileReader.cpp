// stdlib
#include <fstream>

#include <Support/IO/FileReader.h>
#include <Support/Result.h>

#include <llvm/Support/raw_ostream.h>

Result<std::string> getFileContent(std::filesystem::path path) {
  std::string content;
  std::ifstream file(path);
  RET_ON_FALSE(file.is_open(),
               "Unable to open file " + path.filename().string());
  content.assign((std::istreambuf_iterator<char>(file)),
                 std::istreambuf_iterator<char>());
  file.close();
  return content;
}