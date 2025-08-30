#include "Support/Result.h"
#include <AST/ASTContext.h>
#include <Analyzer/Analyzer.h>
#include <Frontend/Compiler.h>
#include <Lexer/Lexer.h>
#include <Parser/Parser.h>

// Gets called by the main with the action that should run.
//  - It creats the ASTContext, the Analyzer and the parser objects.
//  - It then passes a pointer ASTContext to the Analyzer and the parser.
//  - It then tells the parser to start parsing, passing the Analyzer to the
//  parser.
//  - Once finished, the ASTContext contains the AST.
//  - Then the action is called with the ASTContext.
// - The action will act on the AST and perform the action it defines.

namespace Frontend::Compiler {

Error Compiler(Action::Action *action, const std::filesystem::path &fileInput) {

  // Create ASTContext
  auto astContext = AST::Context::ASTContext();

  // Create Analyzer with ASTContext
  auto analyzer = Analyzer::Analyzer(astContext);

  // Get fileInput content and create the Lexer with it.
  auto fileContent = getFileContent(fileInput);
  RET_ON_FAILURE(fileContent, "Failed to get file.");

  auto lexer = Lexing::Lexer(*fileContent);

  // Create Parser with ASTContext, Analyzer and lexer.
  auto parser = Parser::Parser(astContext, analyzer, lexer);

  // Run Parser.Parse()
  RET_ON_FAILURE(parser.Parse(), "Failed to parse.");

  // Run action.ActOn(ASTContext)
  return action->ActOn(astContext);
}

} // namespace Frontend::Compiler