// stdlib
#include <fstream>
#include <iostream>

#include <Support/IO/FileReader.h>

std::string getFileContent(std::filesystem::path path) {
  std::string content;
  std::ifstream file(path);
  if (file.is_open()) {
    content.assign((std::istreambuf_iterator<char>(file)),
                   std::istreambuf_iterator<char>());
    file.close();
  } else {
    std::cerr << "Unable to open file\n";
  }
  return content;
}

args parseArgs(int argc, char *argv[]) {
  args parseArgs;
  for (int i = 0; i < argc; ++i) {
    auto argument = argv[i];
    if (std::string(argument) == "-i") {
      if (i + 1 < argc) {
        parseArgs.input = argv[i + 1];
      } else {
        std::cerr << "'-i' with no argument.\n";
        exit(1);
      }
    }
  }
  if (parseArgs.input.empty()) {
    std::cerr << "No input file provided with '-i'.\n";
    exit(1);
  }
  return parseArgs;
}