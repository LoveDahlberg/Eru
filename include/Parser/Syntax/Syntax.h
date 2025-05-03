
#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <Lexer/Lexer.h>
#include <Lexer/Tokens.h>

#include <AST/CompilationUnit.h>
#include <AST/Types.h>

#include <Support/Constants.h>

using namespace Lexing;

namespace Parser::Syntax {

struct syntaxItems {
  syntaxItems(Lexer &lexer)
      : context(new llvm::LLVMContext()), module(new llvm::Module("", *context)),
        lexer(lexer) {}
private:
  llvm::LLVMContext* context;
public:
  llvm::Module* module;
  Lexer lexer;
  AST::CompilationUnit compilationUnit;
};


inline void skipUntilNotNewline(syntaxItems &items) {
  if (items.lexer.getCurrentToken().type != Lexing::TokenType::NEWLINE) {
    return;
  }

  int loopCounter = 0;
  while (items.lexer.generateNextToken().type == Lexing::TokenType::NEWLINE) {
    if (items.lexer.getCurrentToken().type == Lexing::TokenType::END_OF_FILE) {
      return;
    }
    if (loopCounter++ > loopLimit) {
      return;
    }
  }
}

} // namespace Parser