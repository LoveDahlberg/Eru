
#pragma once

#include <AST/Function.h>
#include <AST/VariableDeclaration.h>

#include <Support/Templates.h>

namespace AST {

// TODO add directive
using CompilationUnitVariant = std::variant<VariableDeclaration::VariableDeclaration*, Function::Function*>;

/// Concept that the given CompilationUnit is:
/// 1. A pointer.
/// 2. One of the variants in CompilationUnitVariant.
template <typename T>
concept ValidCompilationUnitType =
    std::is_pointer_v<T> && is_one_of_variant<T, CompilationUnitVariant>::value;

struct CompilationUnit {

  template <ValidCompilationUnitType CompilationUnitItem>
  void AddCompilationUnitItems(CompilationUnitItem construct) {
    compilationUnitItems.push_back(construct);
  }

  std::vector<CompilationUnitVariant>
      compilationUnitItems;
};

} // namespace AST