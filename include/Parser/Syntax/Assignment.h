#pragma once

#include <Parser/Syntax/Syntax.h>
#include <Parser/Syntax/VariableDeclaration.h>

#include <AST/Assignment.h>

using assignmentAST = ::AST::Assignment::Assignment;

namespace Parser::Syntax::Assignment {
std::optional<assignmentAST *>
ParseAssignment(syntaxItems &items,
                variableDeclarationAST *variableDeclaration = nullptr);
}