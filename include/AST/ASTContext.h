#pragma once

#include <AST/CompilationUnit.h>

namespace AST::Context {

struct ASTContext {
  CompilationUnit *compilationUnit;

  ASTContext(CompilationUnit *compilationUnit)
      : compilationUnit(compilationUnit) {}

  ASTContext() : compilationUnit(new CompilationUnit()) {}

  CompilationUnit *getAST() { return compilationUnit; }

  
};

} // namespace AST::Context