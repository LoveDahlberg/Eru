
#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <Lexer/Lexer.h>
#include <Lexer/Tokens.h>

#include <AST/ASTContext.h>
#include <AST/CompilationUnit.h>
#include <AST/Types.h>

#include <Analyzer/Analyzer.h>

#include <Support/Constants.h>

using namespace Lexing;

namespace Parser {

struct Parser {
  Parser(AST::Context::ASTContext &astContext, Analyzer::Analyzer &analyzer,
         Lexer &lexer)
      : context(new llvm::LLVMContext()), astContext(astContext),
        analyzer(analyzer), module(new llvm::Module("", *context)),
        lexer(lexer) {}

  bool Parse();

private:
  llvm::LLVMContext *context;

public:
  AST::Context::ASTContext &astContext;
  Analyzer::Analyzer &analyzer;
  llvm::Module *module;
  Lexer lexer;
};

inline void skipUntilNotNewline(Parser &items) {
  if (items.lexer.getCurrentToken().type != TokenType::NEWLINE) {
    return;
  }

  int loopCounter = 0;
  while (items.lexer.generateNextToken().type == TokenType::NEWLINE) {
    if (items.lexer.getCurrentToken().type == TokenType::END_OF_FILE) {
      return;
    }
    if (loopCounter++ > loopLimit) {
      return;
    }
  }
}

} // namespace Parser