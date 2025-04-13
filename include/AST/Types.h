
#pragma once

#include "llvm/IR/Value.h"
#include <AST/AST.h>

// stl
#include <cstdint>

namespace AST::Types {

// TODO, these should NOT have a vaule, this is just the declaration of the TYPE
// itself. Its the literals that hold the actual value.

// struct Type {
  // virtual llvm::Type *codegen(llvm::Module* module) = 0;
  // llvm::Value* 
// };

// class Int : public Type {
// public:
//   Int() = default;

//   // llvm::Type *codegen(llvm::Module* module);
// };

// class SingedInt32 : public Type {
// public:
//   SingedInt32() = default;
//   SingedInt32(int32_t value) : value(value) {}

//   // llvm::Type *codegen(llvm::Module* module);

// private:
//   int32_t value;
// };

// class UnsignedInt32 : public Type {
// public:
//   UnsignedInt32() = default;
//   UnsignedInt32(uint32_t value) : value(value) {}

//   llvm::Type *codegen(llvm::Module* module);

// private:
//   uint32_t value;
// };

// class Bool : public Type {
// public:
//   Bool() = default;
//   Bool(bool value) : value(value) {}

//   llvm::Type *codegen(llvm::Module* module);

// private:
//   bool value;
// };

// class String : public Type {
// public:
//   String() = default;
//   String(std::string value) : value(value) {}

//   llvm::Type *codegen(llvm::Module* module);

// private:
//   std::string value;
// };

// class Char : public Type {
// public:
//   Char() = default;
//   Char(char value) : value(value) {}

//   llvm::Type *codegen(llvm::Module* module);

// private:
//   char value;
// };

// class Identifier : public Type {
// public:
//   Identifier(std::string name) : name(name) {}
//   llvm::Type *codegen(llvm::Module* module);

// private:
//   std::string name;
// };

} // namespace AST::Types