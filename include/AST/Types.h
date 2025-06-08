#pragma once

// stl
#include <string>

namespace AST::Types {

// TODO Types might not be the best way to name this. Its a collection of
// targets to be used with std::variant.

enum Types {
  NONE,
  INT,
  SINT32,
  UINT32,
  BOOl,
  CHAR,
  STRING,
};

struct NamedIdentifier {
  std::string value;
};
struct StringLiteral {
  std::string value;
};
struct IntegerLiteral {
  std::string value;
};

} // namespace AST::Types