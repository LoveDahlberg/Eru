#include "AST/Expression.h"
#include <Parser/Parser.h>

namespace Parser {

std::optional<AST::Expression::Operand> Parser::ParseOperand() {
  switch (lexer.getCurrentToken().type) {

  // Identifier or function call
  case TokenType::IDENTIFER: {
    auto identifier = ParseIdentifier();
    if (identifier.hasFailed) {
      // err
      return std::nullopt;
    }

    // Function call
    if (lexer.getCurrentToken().type == TokenType::LEFT_PARENTHESIS) {
      auto functionCall = ParseFunctionCall(*identifier);
      if (!functionCall) {
        // err
        return std::nullopt;
      }
      return AST::Expression::Operand(*functionCall);
    }
    // Identifier
    return AST::Expression::Operand(AST::Types::NamedIdentifier(*identifier));
  }

  case TokenType::STRING_LITERAL: {
    auto literal = ParseLiteral();
    if (!literal) {
      // err
      return std::nullopt;
    }
    return AST::Expression::Operand(AST::Types::StringLiteral(*literal));
  }

  case TokenType::INTEGER_LITERAL: {
    auto literal = ParseLiteral();
    if (!literal) {
      // err
      return std::nullopt;
    }
    return AST::Expression::Operand(AST::Types::IntegerLiteral(*literal));
  }

  default: {
    break;
  }
  }
  // err
  return std::nullopt;
}

std::optional<AST::Expression::ExpressionUnit *>
Parser::ParseExpressionUnit(bool firstUnit) {

  auto unit = new AST::Expression::ExpressionUnit();

  if (!firstUnit) {
    // TODO create proper operator map and category.
    switch (lexer.getCurrentToken().type) {
    case TokenType::AND:
    case TokenType::OR: {
      unit->operation = AST::Expression::TokenToBooleanOperator.at(
          lexer.getCurrentToken().type);
      lexer.generateNextToken();
      break;
    }

    case TokenType::PLUS:
    case TokenType::MINUS: {
      unit->operation = AST::Expression::TokenToArithmeticOperator.at(
          lexer.getCurrentToken().type);
      lexer.generateNextToken();
      break;
    }

    default: {
      // err
      return std::nullopt;
    }
    }
  }

  auto operand = ParseOperand();
  if (!operand) {
    // err
    return std::nullopt;
  }

  unit->operand = *operand;

  return unit;
}

std::optional<AST::Expression::Expression *> Parser::ParseExpression() {

  auto expression = new AST::Expression::Expression();

  bool firstIteration = true;
  int loopCounter = 0;
  do {
    auto target = ParseExpressionUnit(firstIteration);
    if (!target) {
      // err
      return std::nullopt;
    }

    firstIteration = false;

    expression->addExpressionUnit(*target);

    // TODO create proper operator map and category.
    auto nextTokenType = lexer.getCurrentToken().type;
    // Expression is over if next token isn't a operator
    if (nextTokenType != TokenType::PLUS && nextTokenType != TokenType::MINUS &&
        nextTokenType != TokenType::OR && nextTokenType != TokenType::AND) {
      break;
    }
  } while (loopCounter++ < loopLimit);

  return expression;
}

} // namespace Parser