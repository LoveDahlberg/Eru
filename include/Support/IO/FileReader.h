#pragma once

#include <filesystem>

#include <Support/Result.h>

Result<std::string> getFileContent(std::filesystem::path path);