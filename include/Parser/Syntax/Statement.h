#pragma once

#include <Parser/Syntax/Syntax.h>

#include <AST/Statement.h>

using statementAST = ::AST::Statement::Statement;

namespace Parser::Syntax::Statement {

  std::optional<statementAST *> ParseStatement(syntaxItems &items);

}