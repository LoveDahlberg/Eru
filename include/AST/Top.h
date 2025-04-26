
#pragma once

#include <AST/AST.h>
#include <AST/Declaration.h>
#include <AST/Function.h>

#include <llvm/IR/Value.h>

// stl
#include <type_traits>

namespace AST {

// TODO add directive
// TODO try to use inheritence instead of typechecking like this

template <typename T>
concept ValidTopType =
    std::is_pointer_v<T> &&
    (std::is_base_of_v<Declaration::Declaration, std::remove_pointer_t<T>> ||
     std::is_same_v<T, Function::Function>);

struct Top {
  std::vector<llvm::Value *> codegen(llvm::Module &module);

  template <typename topConstruct>
    requires ValidTopType<topConstruct>
  void AddTopConstruct(topConstruct construct) {
    topConstructs.push_back(construct);
  }

  std::vector<AST *>& GetTopConstruct() {
    return topConstructs;
  }

private:
  // Declarations, directives and functions
  std::vector<AST *> topConstructs;
};

} // namespace AST