#pragma once

#include <Parser/Syntax/Syntax.h>
#include <AST/VariableDeclaration.h>

using variableDeclarationAST = ::AST::VariableDeclaration::VariableDeclaration;

namespace Parser::Syntax::VariableDeclaration {

  std::optional<variableDeclarationAST *>
  ParseVariableDeclaration(syntaxItems &items);

}