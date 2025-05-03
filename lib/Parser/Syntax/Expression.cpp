
#include <Parser/Syntax/Expression.h>
#include <Parser/Syntax/Function.h>
#include <Parser/Syntax/Identifier.h>
#include <Parser/Syntax/Literal.h>

namespace Parser::Syntax::Expression {

std::optional<Operand> ParseOperand(syntaxItems &items) {
  switch (items.lexer.getCurrentToken().type) {

  // Identifier or function call
  case TokenType::IDENTIFER: {
    auto identifier = Identifier::ParseIdentifier(items);
    if (!identifier) {
      // err
      return std::nullopt;
    }

    // Function call
    if (items.lexer.getCurrentToken().type == TokenType::LEFT_PARENTHESIS) {
      auto functionCall = Function::ParseFunctionCall(items, *identifier);
      if (!functionCall) {
        // err
        return std::nullopt;
      }
      return Operand(*functionCall);
    }
    // Identifier
    return Operand(AST::Types::NamedIdentifier(*identifier));
  }

  case TokenType::STRING_LITERAL: {
    auto literal = Literal::ParseLiteral(items);
    if (!literal) {
      // err
      return std::nullopt;
    }
    return Operand(AST::Types::StringLiteral(*literal));
  }

  case TokenType::INTEGER_LITERAL: {
    auto literal = Literal::ParseLiteral(items);
    if (!literal) {
      // err
      return std::nullopt;
    }
    return Operand(AST::Types::IntegerLiteral(*literal));
  }

  default: {
    break;
  }
  }
  // err
  return std::nullopt;
}

std::optional<ExpressionUnit *> ParseExpressionUnit(syntaxItems &items) {

  auto operand = ParseOperand(items);
  if (!operand) {
    // err
    return std::nullopt;
  }

  auto unit = new ExpressionUnit();
  unit->operand = *operand;

  // skipUntilNotNewline(items);

  // TODO create proper operator map and category.
  switch (items.lexer.getCurrentToken().type) {
  case TokenType::AND:
  case TokenType::OR: {
    unit->operation =
        TokenToBooleanOperator.at(items.lexer.getCurrentToken().type);
    items.lexer.generateNextToken();
    break;
  }

  case TokenType::PLUS:
  case TokenType::MINUS: {
    unit->operation =
        TokenToArithmeticOperator.at(items.lexer.getCurrentToken().type);
    items.lexer.generateNextToken();
    break;
  }

  default: {
    // No operation after operand
    break;
  }
  }

  return unit;
}

std::optional<expressionAST *> ParseExpression(syntaxItems &items) {

  auto expression = new expressionAST();

  int loopCounter = 0;
  do {
    auto target = ParseExpressionUnit(items);
    if (!target) {
      // err
      return std::nullopt;
    }

    expression->addExpressionUnit(*target);

    // Expression is over if no extra operand was parsed at the end.
    if ((*target)->operation == std::nullopt) {
      break;
    }
  } while (loopCounter++ < loopLimit);

  return expression;
}

} // namespace Parser::Syntax::Expression