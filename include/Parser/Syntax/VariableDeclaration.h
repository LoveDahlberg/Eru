#pragma once

#include <AST/VariableDeclaration.h>
#include <Parser/Syntax/Syntax.h>

using namespace ::AST::VariableDeclaration;
using variableDeclarationAST = ::AST::VariableDeclaration::VariableDeclaration;

namespace Parser::Syntax::VariableDeclaration {

std::optional<Variable *> ParseVariable(syntaxItems &items);

}