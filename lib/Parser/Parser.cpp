#include <Parser/Parser.h>

namespace Parser {

bool Parser::Parse() { 
  // This is the entry point for parsing. 
  // It is supposd to call the top ParseCompilationUnit which completes the 
  // AST using the analyzer, lexer and ASTContext

  // But before doing this, figure out how to separate the codegen from the AST itself.
  // Maybe it just works fine by moving all codegen functions to be standalone and then
  // having them call the next depending on the type.
  

  return true;
}

} // namespace Parser
