#pragma once

#include <Parser/Parser.h>
#include <Parser/VariableDeclaration.h>

#include <AST/Assignment.h>

using assignmentAST = ::AST::Assignment::Assignment;

namespace Parser::Assignment {
std::optional<assignmentAST *>
ParseAssignment(Parser &items,
  Variable *variable = nullptr);
}