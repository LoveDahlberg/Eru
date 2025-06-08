#pragma once

#include <Parser/Parser.h>

#include <AST/Expression.h>

using namespace ::AST::Expression;
using expressionAST = ::AST::Expression::Expression;

namespace Parser::Expression {
std::optional<expressionAST *> ParseExpression(Parser &items);

std::optional<ExpressionUnit *> ParseExpressionUnit(Parser &items);

} // namespace Parser::Expression