#pragma once

#include <AST/ASTContext.h>

namespace Analyzer {

class Analyzer {
  AST::Context::ASTContext &astContext;

public:
  Analyzer(AST::Context::ASTContext &astContext) : astContext(astContext) {}
};

} // namespace Analyzer