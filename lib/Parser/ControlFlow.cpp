
#include <Parser/ControlFlow.h>
#include <Parser/Expression.h>
#include <Parser/Function.h>
#include <Parser/Statement.h>

namespace Parser::Controlflow {

std::optional<ConditionalBranch *> ParseConditionalBranch(ParserItems &items,
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

    branch->addExpression(&*expression);
  }

  auto block = Function::ParseBlock(items);
  if (!block) {
    // err
    return std::nullopt;
  }

  branch->addBlock(&*block);
  return branch;
}

std::optional<ConditionalBranchingGroup *>
ParseConditionalBranchingGroup(ParserItems &items) {
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
} // namespace Parser::Controlflow