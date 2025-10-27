#pragma once

#include <filesystem>

#include <AST/CompilationUnit.h>

namespace AST::Context {

struct ASTContext {
  ASTContext(CompilationUnit *compilationUnit,
             const std::filesystem::path inputFile = {})
      : compilationUnit(compilationUnit), inputFile(inputFile) {}

  ASTContext(const std::filesystem::path inputFile = {})
      : compilationUnit(new CompilationUnit()), inputFile(inputFile) {}

  CompilationUnit *getAST() { return compilationUnit; }

  CompilationUnit *compilationUnit;
  std::filesystem::path inputFile;
};

} // namespace AST::Context