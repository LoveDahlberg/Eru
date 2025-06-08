#include <Parser/Parser.h>

namespace Parser {

std::optional<AST::Controlflow::ConditionalBranch *>
Parser::ParseConditionalBranch(bool start) {

  // TODO could refactor this to be more readable.
  if (start) {
    if (lexer.getCurrentToken().type != TokenType::IF) {
      // err
      return std::nullopt;
    }
  } else {
    if (lexer.getCurrentToken().type != TokenType::ELIF &&
        lexer.getCurrentToken().type != TokenType::ELSE) {
      // err
      return std::nullopt;
    }
  }

  bool isNotElse = lexer.getCurrentToken().type != TokenType::ELSE;

  // Eat the if, elif or else
  lexer.generateNextToken();

  auto branch = new AST::Controlflow::ConditionalBranch();

  if (isNotElse) {

    if (lexer.getCurrentToken().type != TokenType::LEFT_PARENTHESIS) {
      // err
      return std::nullopt;
    }

    // Eat the (
    lexer.generateNextToken();

    auto expression = ParseExpression();
    if (!expression) {
      // err
      return std::nullopt;
    }

    if (lexer.getCurrentToken().type != TokenType::RIGHT_PARENTHESIS) {
      // err
      return std::nullopt;
    }
    // Eat the )
    lexer.generateNextToken();

    branch->addExpression(&*expression);
  }

  auto block = ParseBlock();
  if (!block) {
    // err
    return std::nullopt;
  }

  branch->addBlock(&*block);
  return branch;
}

std::optional<AST::Controlflow::ConditionalBranchingGroup *>
Parser::ParseConditionalBranchingGroup() {
  std::vector<AST::Controlflow::ConditionalBranch *> conditionalChain;

  bool start = true;
  Token lookaheadToken;
  do {
    skipUntilNotNewline();
    auto ConditionalBranch = ParseConditionalBranch(start);
    if (!ConditionalBranch) {
      // err
      return std::nullopt;
    }
    start = false;
    conditionalChain.push_back(*ConditionalBranch);

    lookaheadToken = lexer.lookaheadTokenNotNewline();
  } while (lookaheadToken.type == TokenType::ELIF ||
           lookaheadToken.type == TokenType::ELSE);

  return new AST::Controlflow::ConditionalBranchingGroup(conditionalChain);
}
} // namespace Parser