#pragma once

#include <Parser/Parser.h>

#include <AST/Statement.h>

using statementAST = ::AST::Statement::Statement;

namespace Parser::Statement {

  std::optional<statementAST *> ParseStatement(ParserItems &items);

}