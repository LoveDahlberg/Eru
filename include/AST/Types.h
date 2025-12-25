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
  Type() : dataType(NONE), isPointer(false) {}

  Type(DataType dataType) : dataType(dataType), isPointer(false) {}

  DataType dataType;
  bool isPointer = false;

  bool operator==(const Type &compare) const = default;
  bool operator!=(const Type &compare) const = default;

  const std::string &toString() const { return typeToString.at(dataType); }
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