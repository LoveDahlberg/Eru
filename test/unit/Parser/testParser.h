#include <gtest/gtest.h>

#include <Lexer/Lexer.h>
#include <Lexer/Tokens.h>

#include <Parser/Parser.h>

struct items {
  items(std::string &stream)
      : lexer(stream), astContext(), analyzer(astContext), result() {}

  Lexing::Lexer lexer;
  AST::Context::ASTContext astContext;
  Analyzer::Analyzer analyzer;

  bool success = true;
  std::optional<Error> result;
};

inline items RunParser(std::string &stream, bool expectSuccess = true) {

  stream += EOF;
  auto item = items(stream);

  Parser::Parser parser(item.astContext, item.analyzer, item.lexer);

  auto result = parser.Parse();

  // Error if I expect success but it fails. 
  if (expectSuccess && result.hasFailed ||
      !expectSuccess && !result.hasFailed) {
    std::cout << "\nFailure, printing description trace:\n";
    for (auto reason : result.failureDescription) {
      std::cout << reason << "\n";
    }
    std::cout << "Printing code trace:\n";
    std::cout << result.codeSnippet << "\n";

    std::cout << "\n\n";

    // item.lexer.

    item.success = false;
  }
  item.result = result;
  return item;
}
