#pragma once

#include <Parser/Syntax/Syntax.h>

namespace Parser::Syntax::Type {

std::optional<llvm::Type *> ParseType(syntaxItems &items);

}