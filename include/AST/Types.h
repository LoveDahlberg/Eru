
#pragma once

#include <AST/AST.h>

// stl
#include <cstdint>

namespace AST::Types {

class Type : public AST {
  llvm::Value *codegen() { return nullptr; }
};

class Int : public Type {
public:
  Int(int value) : value(value) {}

  llvm::Value *codegen() { return nullptr; }

private:
  int value;
};

class SingedInt32 : public Type {
public:
  SingedInt32(int32_t value) : value(value) {}

  llvm::Value *codegen() { return nullptr; }

private:
  int32_t value;
};

class UnsignedInt32 : public Type {
public:
  UnsignedInt32(uint32_t value) : value(value) {}

  llvm::Value *codegen() { return nullptr; }

private:
  uint32_t value;
};

class Bool : public Type {
public:
  Bool(bool value) : value(value) {}

  llvm::Value *codegen() { return nullptr; }

private:
  bool value;
};

class String : public Type {
public:
  String(std::string value) : value(value) {}

  llvm::Value *codegen() { return nullptr; }

private:
  std::string value;
};

class Char : public Type {
public:
  Char(char value) : value(value) {}

  llvm::Value *codegen() { return nullptr; }

private:
  char value;
};

class Identifier : public Type {
public:
  Identifier(std::string name) : name(name) {}

  llvm::Value *codegen() { return nullptr; }

private:
  std::string name;
};

} // namespace AST::Types