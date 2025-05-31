#pragma once

#include <Parser/Parser.h>

#include <AST/Expression.h>

using namespace ::AST::Expression;
using expressionAST = ::AST::Expression::Expression;

namespace Parser::Expression {
std::optional<expressionAST *> ParseExpression(ParserItems &items);

std::optional<ExpressionUnit *> ParseExpressionUnit(ParserItems &items);

} // namespace Parser::Expression