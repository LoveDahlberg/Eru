#pragma once

// stl
#include <string>
#include <unordered_map>

namespace AST::Types {

// TODO Types might not be the best way to name this. Its a collection of
// targets to be used with std::variant.

enum DataType {
  NONE,
  INT,
  SINT32,
  UINT32,
  BOOl,
  CHAR,
  STRING,
};

namespace {

static std::unordered_map<DataType, std::string> typeToString{
    {NONE, "void"}, {INT, "int"},   {SINT32, "sint32"},
    {BOOl, "bool"}, {CHAR, "char"}, {STRING, "string"},
};

}

struct Type {
  Type() : dataType(NONE), isPointer(false), pointerDepth(0) {}

  Type(DataType dataType, bool isPointer = false, int pointerDepth = 0)
      : dataType(dataType), isPointer(isPointer), pointerDepth(pointerDepth) {}

  DataType dataType;
  bool isPointer;
  int pointerDepth;

  bool operator==(const Type &compare) const = default;
  bool operator!=(const Type &compare) const = default;

  const std::string toPrintableString() const {
    return " '" + typeToString.at(dataType) + std::string(pointerDepth, '&') +
           "' ";
  }
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