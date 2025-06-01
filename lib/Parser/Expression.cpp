
#include <Parser/Expression.h>
#include <Parser/Function.h>
#include <Parser/Identifier.h>
#include <Parser/Literal.h>

namespace Parser::Expression {

std::optional<Operand> ParseOperand(Parser &items) {
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

std::optional<ExpressionUnit *> ParseExpressionUnit(Parser &items,
                                                    bool firstUnit) {

  auto unit = new ExpressionUnit();

  if (!firstUnit) {
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
      // err
      return std::nullopt;
    }
    }
  }

  auto operand = ParseOperand(items);
  if (!operand) {
    // err
    return std::nullopt;
  }

  unit->operand = *operand;

  return unit;
}

std::optional<expressionAST *> ParseExpression(Parser &items) {

  auto expression = new expressionAST();

  bool firstIteration = true;
  int loopCounter = 0;
  do {
    auto target = ParseExpressionUnit(items, firstIteration);
    if (!target) {
      // err
      return std::nullopt;
    }

    firstIteration = false;

    expression->addExpressionUnit(*target);

    // TODO create proper operator map and category.
    auto nextTokenType = items.lexer.getCurrentToken().type;
    // Expression is over if next token isn't a operator
    if (nextTokenType != TokenType::PLUS && nextTokenType != TokenType::MINUS &&
        nextTokenType != TokenType::OR && nextTokenType != TokenType::AND) {
      break;
    }
  } while (loopCounter++ < loopLimit);

  return expression;
}

} // namespace Parser::Expression