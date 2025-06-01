#pragma once

#include <Parser/Parser.h>

namespace Parser::Literal {
  std::optional<std::string> ParseLiteral(Parser &items);
}