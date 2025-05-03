
#include <Parser/Syntax/ControlFlow.h>
#include <Parser/Syntax/Expression.h>
#include <Parser/Syntax/Statement.h>

namespace Parser::Syntax::Controlflow {

std::optional<ConditionalBranch *> ParseConditionalBranch(syntaxItems &items,
                                                          bool start = false) {

  // TODO could refactor this to be more readable.
  if (start) {
    if (items.lexer.getCurrentToken().type != TokenType::IF) {
      // err
      return std::nullopt;
    }
  } else {
    if (items.lexer.getCurrentToken().type != TokenType::ELIF &&
        items.lexer.getCurrentToken().type != TokenType::ELSE) {
      // err
      return std::nullopt;
    }
  }

  bool isNotElse = items.lexer.getCurrentToken().type != TokenType::ELSE;

  // Eat the if, elif or else
  items.lexer.generateNextToken();

  auto branch = new ConditionalBranch();

  if (isNotElse) {

    if (items.lexer.getCurrentToken().type != TokenType::LEFT_PARENTHESIS) {
      // err
      return std::nullopt;
    }

    // Eat the (
    items.lexer.generateNextToken();

    auto expression = Expression::ParseExpression(items);
    if (!expression) {
      // err
      return std::nullopt;
    }

    if (items.lexer.getCurrentToken().type != TokenType::RIGHT_PARENTHESIS) {
      // err
      return std::nullopt;
    }
    // Eat the )
    items.lexer.generateNextToken();

    branch->addExpression(*expression);
  }

  skipUntilNotNewline(items);

  if (items.lexer.getCurrentToken().type != TokenType::LEFT_CURLY_BRACE) {
    // err
    return std::nullopt;
  }

  // Eat the {
  items.lexer.generateNextToken();

  auto statement = Statement::ParseStatement(items);
  if (!statement) {
    // err
    return std::nullopt;
  }

  if (items.lexer.getCurrentToken().type != TokenType::RIGHT_CURLY_BRACE) {
    // err
    return std::nullopt;
  }

  // Eat the }
  items.lexer.generateNextToken();

  branch->addStatement(*statement);
  return branch;
}

std::optional<ConditionalBranchingGroup *>
ParseConditionalBranchingGroup(syntaxItems &items) {
  std::vector<ConditionalBranch *> conditionalChain;

  bool start = true;
  Token lookaheadToken;
  do {
    skipUntilNotNewline(items);
    auto ConditionalBranch = ParseConditionalBranch(items, start);
    if (!ConditionalBranch) {
      // err
      return std::nullopt;
    }
    start = false;
    conditionalChain.push_back(*ConditionalBranch);

    lookaheadToken = items.lexer.lookaheadTokenNotNewline();
  } while (lookaheadToken.type == TokenType::ELIF ||
           lookaheadToken.type == TokenType::ELSE);

  return new ConditionalBranchingGroup(conditionalChain);
}
} // namespace Parser::Syntax::Controlflow