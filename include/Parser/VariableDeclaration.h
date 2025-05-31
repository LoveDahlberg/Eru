#pragma once

#include <AST/VariableDeclaration.h>
#include <Parser/Parser.h>

using namespace ::AST::VariableDeclaration;
using variableDeclarationAST = ::AST::VariableDeclaration::VariableDeclaration;

namespace Parser::VariableDeclaration {

std::optional<Variable *> ParseVariable(ParserItems &items);

}