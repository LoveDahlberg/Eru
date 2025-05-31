#pragma once

#include <Parser/Parser.h>

namespace Parser::Type {

std::optional<llvm::Type *> ParseType(ParserItems &items);

}