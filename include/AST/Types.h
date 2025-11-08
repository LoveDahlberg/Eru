#pragma once

// stl
#include <string>
#include <unordered_map>

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

static std::unordered_map<Types, std::string> typeToString{
    {NONE, "void"}, {INT, "int"},   {SINT32, "sint32"},
    {BOOl, "bool"}, {CHAR, "char"}, {STRING, "string"},
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