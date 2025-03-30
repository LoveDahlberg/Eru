#pragma once

#include <filesystem>

std::string getFileContent(std::filesystem::path path);

struct args {
  std::string input;
};

args parseArgs(int argc, char *argv[]);