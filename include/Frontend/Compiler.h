#pragma once

#include <Frontend/Actions.h>

namespace Frontend::Compiler {

// TODO: Fileinput should represent all input files.
bool Compiler(Action::Action *action, const std::string &fileInput);
}
