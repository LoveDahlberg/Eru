#pragma once

#include <filesystem>
#include <string>

enum TokenType { endOfFile, identifier, number };

struct Token {
  TokenType type;
  std::string identifier;
  double number;
  std::string rawValue;
};

int getNextCharacter(std::string input);
Token getToken(std::string input);
std::string getFileContent(std::filesystem::path path);