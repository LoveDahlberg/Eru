#include "AST/Expression.h"
#include <Parser/Parser.h>

namespace Parser {

Result<AST::Expression::Operand> Parser::ParseOperand() {
  switch (lexer.getCurrentToken().type) {

  // Identifier or function call
  case TokenType::IDENTIFER: {
    auto identifier = ParseIdentifier();
    RET_ON_FAILURE(identifier, "ParseOperand: failed identifier");

    // Function call
    if (lexer.getCurrentToken() == TokenType::LEFT_PARENTHESIS) {
      auto functionCall = ParseFunctionCall(*identifier);

      RET_ON_FAILURE(functionCall, "ParseOperand: failed function call");

      return AST::Expression::Operand(*functionCall);
    }
    // Identifier
    return AST::Expression::Operand(AST::Types::NamedIdentifier(*identifier));
  }

  case TokenType::STRING_LITERAL: {
    auto literal = ParseLiteral();

    RET_ON_FAILURE(literal, "ParseOperand: failed string literal");

    return AST::Expression::Operand(AST::Types::StringLiteral(*literal));
  }

  case TokenType::INTEGER_LITERAL: {
    auto literal = ParseLiteral();

    RET_ON_FAILURE(literal, "ParseOperand: failed integer literal");

    return AST::Expression::Operand(AST::Types::IntegerLiteral(*literal));
  }

  default: {
    return {"ParseOperand: unexpected token"};
  }
  }
}

// TODO: rewrite this, this is doing the analyzers job at the moment.
Result<AST::Expression::ExpressionUnit *>
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
      return {"ParseExpressionUnit: unexpected operation"};
    }
    }
  }

  auto operand = ParseOperand();
  RET_ON_FAILURE(operand, "ParseExpressionUnit: failed operand");

  unit->operand = *operand;

  return unit;
}

Result<AST::Expression::Expression *> Parser::ParseExpression() {

  auto expression = new AST::Expression::Expression();

  bool firstIteration = true;
  int loopCounter = 0;
  do {
    auto target = ParseExpressionUnit(firstIteration);
    RET_ON_FAILURE(target, "ParseExpression: failed target");

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