#include "AST/Types.h"
#include <AST/VariableDeclaration.h>
#include <Parser/Parser.h>
#include <Support/Result.h>

namespace Parser {

Result<AST::Expression::ConstantOperand> Parser::ParseConstantOperand() {

  if (lexer.getCurrentToken().type != TokenType::STRING_LITERAL &&
      lexer.getCurrentToken().type != TokenType::INTEGER_LITERAL) {
    return FAILURE_CODE(
        "ParseConstantOperand: Expected string or integer literal", lexer);
  }

  auto literal = ParseLiteral();
  RET_ON_FAILURE_CODE(literal, "ParseConstantOperand: failed to parse literal",
                      lexer);

  return lexer.getCurrentToken().type == TokenType::STRING_LITERAL
             ? AST::Expression::ConstantOperand(
                   AST::Types::StringLiteral(*literal))
             : AST::Expression::ConstantOperand(
                   AST::Types::IntegerLiteral(*literal));
}

// TODO properly implement the different operands. Currently its super unclear
// what is done here. How are booleans handled here for example? What is the
// difference between an integer literal and a int data type?
Result<AST::Expression::Operand> Parser::ParseOperand() {
  switch (lexer.getCurrentToken().type) {

  // Identifier or function call
  case TokenType::IDENTIFER: {
    auto identifier = ParseIdentifier();
    RET_ON_FAILURE_CODE(identifier, "ParseOperand: failed identifier", lexer);

    // Function call
    if (lexer.getCurrentToken() == TokenType::LEFT_PARENTHESIS) {
      auto functionCall = ParseFunctionCall(*identifier);

      RET_ON_FAILURE_CODE(functionCall, "ParseOperand: failed function call",
                          lexer);

      return AST::Expression::Operand(*functionCall);
    }
    // Identifier
    return AST::Expression::Operand(AST::Types::NamedIdentifier(*identifier));
  }

  case TokenType::STRING_LITERAL: {
    auto literal = ParseLiteral();

    RET_ON_FAILURE_CODE(literal, "ParseOperand: failed string literal", lexer);

    return AST::Expression::Operand(AST::Types::StringLiteral(*literal));
  }

  case TokenType::INTEGER_LITERAL: {
    auto literal = ParseLiteral();

    RET_ON_FAILURE_CODE(literal, "ParseOperand: failed integer literal", lexer);

    return AST::Expression::Operand(AST::Types::IntegerLiteral(*literal));
  }

  default: {
    return FAILURE_CODE("ParseOperand: unexpected token", lexer);
  }
  }
}

// TODO: rewrite this, this is doing the analyzers job at the moment.
Result<AST::Expression::ExpressionUnit *>
Parser::ParseExpressionUnit(bool firstUnit) {

  auto unit = new AST::Expression::ExpressionUnit();

  if (!firstUnit) {
    RET_ON_FALSE_CODE(tokenTypeToCategory.at(lexer.getCurrentToken().type) ==
                          TokenCategory::OPERATOR,
                      "ParseExpressionUnit: unexpected token category.", lexer);

    RET_ON_FALSE_CODE(
        tokenTypeToOperator.contains(lexer.getCurrentToken().type),
        "ParseExpressionUnit: misconfigured tokenTypeToOperator map.", lexer);

    unit->operation = tokenTypeToOperator.at(lexer.getCurrentToken().type);
    lexer.generateNextToken();
  }

  auto operand = ParseOperand();
  RET_ON_FAILURE_CODE(operand, "ParseExpressionUnit: failed to parse operand",
                      lexer);

  unit->operand = *operand;

  return unit;
}

Result<AST::Expression::Expression *> Parser::ParseExpression() {

  auto expression = new AST::Expression::Expression();

  bool firstIteration = true;
  int loopCounter = 0;
  do {
    auto target = ParseExpressionUnit(firstIteration);
    RET_ON_FAILURE_CODE(target, "ParseExpression: failed target", lexer);

    firstIteration = false;

    expression->addExpressionUnit(*target);

    if (tokenTypeToCategory.at(lexer.getCurrentToken().type) !=
        TokenCategory::OPERATOR) {
      break;
    }
  } while (loopCounter++ < loopLimit);

  RET_ON_FAILURE_CODE(analyzer.expression().ActOn(expression),
                      "ParseExpression: failed to act on expression.", lexer);

  return expression;
}

} // namespace Parser