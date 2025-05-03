
#pragma once

#include <AST/AST.h>
#include <AST/VariableDeclaration.h>
#include <AST/Function.h>

// stl
#include <type_traits>

namespace AST {

// TODO add directive
// TODO try to use inheritence instead of typechecking like this
/// ValidCompilationUnitType accepts any pointer where the class is VariableDeclaration
/// or FunctionDefinition.
template <typename T>
concept ValidCompilationUnitType =
    std::is_pointer_v<T> &&
    (std::is_same_v<VariableDeclaration::VariableDeclaration, std::remove_pointer_t<T>> ||
     std::is_same_v<Function::Function, std::remove_pointer_t<T>>);

struct CompilationUnit : public GeneratingAST {
  std::vector<llvm::Value *> codegen(llvm::Module &module) override;

  template <typename CompilationUnitItem>
    requires ValidCompilationUnitType<CompilationUnitItem>
  void AddCompilationUnitItems(CompilationUnitItem construct) {
    compilationUnitItems.push_back(construct);
  }

  std::vector<AST *> &GetAddCompilationUnitItems() { return compilationUnitItems; }

private:
  // Declarations, directives and functions
  std::vector<AST *> compilationUnitItems;
};

} // namespace AST