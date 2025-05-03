#pragma once

#include <Parser/Syntax/Syntax.h>

#include <AST/Expression.h>

using namespace ::AST::Expression;
using expressionAST = ::AST::Expression::Expression;

namespace Parser::Syntax::Expression {
std::optional<expressionAST *> ParseExpression(syntaxItems &items);

std::optional<ExpressionUnit *> ParseExpressionUnit(syntaxItems &items);

} // namespace Parser::Syntax::Expression