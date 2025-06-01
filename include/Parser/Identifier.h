#pragma once

#include <Parser/Parser.h>

namespace Parser::Identifier {

std::optional<std::string> ParseIdentifier(Parser &items);
}