#include "Lexer/Tokens.h"

namespace Lexing {

bool isTokenTypePartOfCategory(const TokenType &type,
                               const TokenCategory &category) {
  const auto &storedCategories = tokenTypeToCategory.at(type);
  return static_cast<bool>(storedCategories & category);
}

} // namespace Lexing