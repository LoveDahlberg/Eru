
// stl

#include <string>

// include
#include <Lexer/Lexer.h>
#include <Lexer/Tokens.h>
#include <Support/Log.h>
#include <AST/Top.h>

// llvm
#include <llvm/Support/Error.h> // For llvm::Expected

using namespace Lexer;
using namespace AST;

namespace Parser {

constexpr auto topParsingName = "TopParsing";
constexpr auto declarationParsingName = "DeclarationParsing";
constexpr auto directiveParsingName = "DirectiveParsing";
constexpr auto typeParsingName = "TypeParsing";

void printParsing(const char *parsingFunctionName, TokenCategory tokenCategory,
                  Token token) {
  LogError("{}: Unexpected token of:\nCategory '{}'\n Type '{}'\n "
           "Value '{}'\n",
           parsingFunctionName, std::to_string(static_cast<int>(tokenCategory)),
           std::to_string(static_cast<int>(token.type)), token.value);
}

void DirectiveParsing(Tokenizer &tokenizer, const Token &token) {
  switch (token.type) {}
}

void DeclarationParsing(Tokenizer &tokenizer, const Token &token) {
  switch (token.type) {}
}

llvm::Expected<Types::Type> TypeParsing(Tokenizer &tokenizer, const Token &token) {
  
  Types::Type type;
  
  switch (token.type) {
  case TokenType::IDENTIFER:
  case TokenType::INT:
  case TokenType::SIGNED_INT_32:
  case TokenType::UNSIGNED_INT_32:
  case TokenType::BOOl:
  case TokenType::STRING:
  case TokenType::CHAR:
    // Create each unique type
    break;
  default:
    printParsing(typeParsingName, TokenCategory::TYPE, token);
    break;
  }

  return type;
}

Top TopParsing(Tokenizer &tokenizer) {
  Top top{};

  Token token;
  TokenCategory tokenCategory;
  for (;;) {
    token = tokenizer.getToken();
    tokenCategory = tokenTypeToCategory.at(token.type);
    switch (tokenCategory) {
    case TokenCategory::SEPARATOR:
      DirectiveParsing(tokenizer, token);
      continue;
    case TokenCategory::TYPE:
      auto type = TypeParsing(tokenizer, token);
      if
      continue;
    default:
      printParsing(topParsingName, tokenCategory, token);
      break;
    }
    // Break the main loop
    break;
  }

  return top;
}

} // namespace Parser