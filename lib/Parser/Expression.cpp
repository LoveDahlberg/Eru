#include <AST/VariableDeclaration.h>
#include <Parser/Parser.h>
#include <Support/Result.h>

// stdlib
#include <string>

namespace Parser {

Result<AST::Expression::ConstantOperand> Parser::ParseConstantOperand() {

  if (lexer.getCurrentToken() != TokenType::STRING_LITERAL &&
      lexer.getCurrentToken() != TokenType::INTEGER_LITERAL) {
    return FAILURE_CODE(
        "ParseConstantOperand: Expected string or integer literal", lexer);
  }

  auto literal = ParseLiteral();
  RET_ON_FAILURE_CODE(literal, "ParseConstantOperand: failed to parse literal",
                      lexer);

  return lexer.getCurrentToken() == TokenType::STRING_LITERAL
             ? AST::Expression::ConstantOperand(
                   AST::Types::StringLiteral(*literal))
             : AST::Expression::ConstantOperand(
                   AST::Types::IntegerLiteral(*literal));
}

Result<AST::Expression::Operand> Parser::ParseIndirection() {

  // This has to be able to fit plus/minus the pointerIndirectionLimit to not
  // overflow.
  int32_t indirectionStep = 0;
  bool limitReached = false;

  TokenType previousStep = TokenType::NONE;

  int loopCounter = 0;
  do {
    if (lexer.getCurrentToken() == TokenType::AMPERSAND) {
      RET_ON_EQUAL_CODE(
          previousStep, TokenType::AMPERSAND,
          "ParseIndirection: cannot get the address of a temporary.", lexer);

      previousStep = TokenType::AMPERSAND;
      ++indirectionStep;

    } else if (lexer.getCurrentToken() == TokenType::STAR) {
      --indirectionStep;
      previousStep = TokenType::STAR;

    } else {
      break;
    }

    lexer.generateNextToken();

    if (indirectionStep > pointerIndirectionLimit &&
        indirectionStep < -pointerIndirectionLimit) {
      limitReached = true;
    }

  } while (loopCounter++ < loopLimit && !limitReached);

  RET_ON_TRUE_CODE(limitReached,
                   "ParseIndirection: maximum level of pointer indirection "
                   "reached. Limit is plus/miuns '" +
                       std::to_string(pointerIndirectionLimit) + "'.",
                   lexer);

  auto operandIndirection = [&]() {
    if (indirectionStep == 0) {
      return AST::Expression::OperandIndirection::NONE;
    }
    return indirectionStep > 0
               ? AST::Expression::OperandIndirection::GET_ADDRESS
               : AST::Expression::OperandIndirection::GET_VALUE;
  }();

  return AST::Expression::Operand(operandIndirection, abs(indirectionStep));
}

// TODO properly implement the different operands. Currently its super unclear
// what is done here. How are booleans handled here for example? What is the
// difference between an integer literal and a int data type?
Result<AST::Expression::Operand> Parser::ParseOperand() {

  auto operandOrErr = ParseIndirection();

  RET_ON_FAILURE(operandOrErr, "ParseOperand: faield to parse indirection");

  auto operand = *operandOrErr;

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

      RET_ON_NOT_EQUAL_CODE(operand.indirection,
                            AST::Expression::OperandIndirection::NONE,
                            "Cannot perform pointer indirection on values "
                            "directly being returned from function calls. Called function: '" +
                                (*functionCall)->name + "'.",
                            lexer);

      return operand.setOperandKind(*functionCall);
    }
    // Identifier
    return operand.setOperandKind(AST::Types::NamedIdentifier(*identifier));
  }

  case TokenType::STRING_LITERAL: {
    auto literal = ParseLiteral();

    RET_ON_FAILURE_CODE(literal, "ParseOperand: failed string literal", lexer);

    RET_ON_NOT_EQUAL_CODE(
        operand.indirection, AST::Expression::OperandIndirection::NONE,
        "Cannot perform pointer indirection on string literals. Value: '" +
            *literal.value + "'.",
        lexer);

    return operand.setOperandKind(AST::Types::StringLiteral(*literal));
  }

  case TokenType::INTEGER_LITERAL: {
    auto literal = ParseLiteral();

    RET_ON_FAILURE_CODE(literal, "ParseOperand: failed integer literal", lexer);

    RET_ON_NOT_EQUAL_CODE(
        operand.indirection, AST::Expression::OperandIndirection::NONE,
        "Cannot perform pointer indirection on integer literals. Value: '" +
            *literal.value + "'.",
        lexer);

    return operand.setOperandKind(AST::Types::IntegerLiteral(*literal));
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

  RET_ON_FAILURE_CODE(analyzer.expression().ActOnExpression(expression),
                      "ParseExpression: failed to act on expression.", lexer);

  return expression;
}

} // namespace Parser