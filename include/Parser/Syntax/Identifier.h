#pragma once

#include <Parser/Syntax/Syntax.h>

namespace Parser::Syntax::Identifier {

std::optional<std::string> ParseIdentifier(syntaxItems &items);
}