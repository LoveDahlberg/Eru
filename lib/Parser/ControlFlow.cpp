#include <Parser/Parser.h>

namespace Parser {

Result<AST::Controlflow::ConditionalBranch *>
Parser::ParseConditionalBranch(bool start) {

  // TODO could refactor this to be more readable.
  if (start) {
    RET_ON_WRONG_TOKEN(TokenType::IF, "ParseConditionalBranch: Expected if");
  } else {
    if (lexer.getCurrentToken().type != TokenType::ELIF &&
        lexer.getCurrentToken().type != TokenType::ELSE) {
      return FAILURE_CODE("ParseConditionalBranch: Expected elif or else", lexer);
    }
  }

  bool isNotElse = lexer.getCurrentToken().type != TokenType::ELSE;

  // Eat the if, elif or else
  lexer.generateNextToken();

  auto branch = new AST::Controlflow::ConditionalBranch();

  if (isNotElse) {

    RET_ON_WRONG_TOKEN(TokenType::LEFT_PARENTHESIS,
                       "ParseConditionalBranch: Expected (");

    // Eat the (
    lexer.generateNextToken();

    auto expression = ParseExpression();
    RET_ON_FAILURE_CODE(expression, "ParseConditionalBranch: failure in expression", lexer);

    RET_ON_WRONG_TOKEN(TokenType::RIGHT_PARENTHESIS,
                       "ParseConditionalBranch: Expected )");

    // Eat the )
    lexer.generateNextToken();

    // TODO fix this mess
    auto *expr = *expression;
    branch->addExpression(&expr);
  }

  analyzer.PushScope();
  auto block = ParseBlock();
  analyzer.PopScope();

  RET_ON_FAILURE_CODE(block, "ParseConditionalBranch: failure in block", lexer);

  // TODO fix this mess
  auto *br = *block;
  branch->addBlock(&br);
  return branch;
}

Result<AST::Controlflow::ConditionalBranchingGroup *>
Parser::ParseConditionalBranchingGroup() {
  std::vector<AST::Controlflow::ConditionalBranch *> conditionalChain;

  bool start = true;
  Token lookaheadToken;
  do {
    skipUntilNotNewline();
    auto ConditionalBranch = ParseConditionalBranch(start);

    RET_ON_FAILURE_CODE(
        ConditionalBranch,
        "ParseConditionalBranchingGroup: Failed to get conditional branch", lexer);

    start = false;
    conditionalChain.push_back(*ConditionalBranch);

    lookaheadToken = lexer.lookaheadTokenNotNewline();
  } while (lookaheadToken.type == TokenType::ELIF ||
           lookaheadToken.type == TokenType::ELSE);

  return new AST::Controlflow::ConditionalBranchingGroup(conditionalChain);
}
} // namespace Parser