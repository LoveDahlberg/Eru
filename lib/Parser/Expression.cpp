#include <Parser/Parser.h>

namespace Parser {

// TODO properly implement the different operands. Currently its super unclear
// what is done here. How are booleans handled here for example? What is the
// difference between an integer literal and a int data type?
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
    RET_ON_FALSE(tokenTypeToCategory.at(lexer.getCurrentToken().type) ==
                     TokenCategory::OPERATOR,
                 "ParseExpressionUnit: unexpected token category.");

    RET_ON_FALSE(tokenTypeToOperator.contains(lexer.getCurrentToken().type),
                 "ParseExpressionUnit: misconfigured tokenTypeToOperator map.");

    unit->operation = tokenTypeToOperator.at(lexer.getCurrentToken().type);
    lexer.generateNextToken();
  }

  auto operand = ParseOperand();
  RET_ON_FAILURE(operand, "ParseExpressionUnit: failed to parse operand");

  unit->operand = *operand;

  return unit;
}

Result<AST::Expression::Expression *>
Parser::ParseExpression(AST::Types::Types expectedType) {

  auto expression = new AST::Expression::Expression();

  bool firstIteration = true;
  int loopCounter = 0;
  do {
    auto target = ParseExpressionUnit(firstIteration);
    RET_ON_FAILURE(target, "ParseExpression: failed target");

    firstIteration = false;

    expression->addExpressionUnit(*target);

    if (tokenTypeToCategory.at(lexer.getCurrentToken().type) !=
        TokenCategory::OPERATOR) {
      break;
    }
  } while (loopCounter++ < loopLimit);

  RET_ON_FAILURE(analyzer.expression().ActOn(expression, expectedType),
                 "ParseExpression: failed to act on expression.");

  return expression;
}

} // namespace Parser