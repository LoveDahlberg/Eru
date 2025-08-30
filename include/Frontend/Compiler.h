#pragma once

#include <Support/Result.h>
#include <Frontend/Actions.h>

// stl
#include <filesystem>

namespace Frontend::Compiler {

// TODO: Fileinput should represent all input files.
Error Compiler(Action::Action *action, const std::filesystem::path &fileInput);
}
