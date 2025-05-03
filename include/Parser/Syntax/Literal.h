#pragma once

#include <Parser/Syntax/Syntax.h>

namespace Parser::Syntax::Literal {
  std::optional<std::string> ParseLiteral(syntaxItems &items);
}