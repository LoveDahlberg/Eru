#pragma once

#include <Frontend/Actions.h>
#include <Support/IO/File.h>
#include <Support/Result.h>

namespace Frontend::Compiler {

Error Compile(Action::CompilationAction *action, Support::IO::Files &files);

} // namespace Frontend::Compiler
