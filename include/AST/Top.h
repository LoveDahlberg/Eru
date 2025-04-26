
#pragma once

#include <AST/AST.h>
#include <AST/Declaration.h>
#include <AST/Function.h>

// stl
#include <type_traits>

namespace AST {

// TODO add directive
// TODO try to use inheritence instead of typechecking like this
/// ValidTopType accepts any pointer where the class inherits from Declaration
/// or exactly FunctionDefinition.
template <typename T>
concept ValidTopType =
    std::is_pointer_v<T> &&
    (std::is_base_of_v<Declaration::Declaration, std::remove_pointer_t<T>> ||
     std::is_same_v<Function::FunctionDefinition, std::remove_pointer_t<T>>);

struct Top : public GeneratingAST {
  std::vector<llvm::Value *> codegen(llvm::Module &module) override;

  template <typename topConstruct>
    requires ValidTopType<topConstruct>
  void AddTopConstruct(topConstruct construct) {
    topConstructs.push_back(construct);
  }

  std::vector<AST *> &GetTopConstruct() { return topConstructs; }

private:
  // Declarations, directives and functions
  std::vector<AST *> topConstructs;
};

} // namespace AST