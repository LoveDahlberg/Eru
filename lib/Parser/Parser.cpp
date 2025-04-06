
// stl

#include "AST/Declaration.h"
#include "AST/Types.h"
#include <memory>
#include <optional>
#include <string>

// include
#include <AST/Top.h>
#include <Lexer/Lexer.h>
#include <Lexer/Tokens.h>
#include <Support/Log.h>

// llvm
#include <llvm/Support/Error.h>
#include <vector>

using namespace Lexer;
using namespace AST;

namespace Parser {

// TODO either move this to a class or make the API here better
static Token token;
static TokenCategory tokenCategory;
static Tokenizer *tokenizer;

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

bool ParseDirective(Top &top) {
  return false;
}

std::optional<Types::Identifier> ParseIdentifier() {
  token = tokenizer->getToken();
  return std::nullopt;
}

std::optional<Types::Type> ParseType() {
  token = tokenizer->getToken();
  switch (token.type) {
  case TokenType::INT:
    return Types::Int(std::stoi(token.value));
  case TokenType::IDENTIFER:
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

  return std::nullopt;
}

std::optional<std::vector<Declaration::Declaration>> ParseParameters() {
  token = tokenizer->getToken();
  return std::nullopt;
}

bool ParseFunctionDefinition(Top &top, const Types::Type &type,
                             const Types::Identifier &identifier,
                             std::vector<Declaration::Declaration> parameters) {
  return false;
}

bool ParseFunctionDefinitionOrDeclaration(Top &top, const Types::Type &type,
                                          const Types::Identifier &identifier) {
  // Eat the (
  token = tokenizer->getToken();
  auto paramaters = ParseParameters();
  if (!paramaters) {
    // err
    return false;
  }

  if (token.type != TokenType::RIGHT_PARENTHESIS) {
    // err
    return false;
  }

  // Eat the )
  token = tokenizer->getToken();
  if (token.type == TokenType::NEWLINE) {
    top.declarations.emplace_back(
        Declaration::FunctionDeclaration(type, identifier, *paramaters));
    return true;
  }

  return ParseFunctionDefinition(top, type, identifier, *paramaters);
}

bool ParseDeclarationOrFunction(Top &top) {
  auto type = ParseType();
  if (!type) {
    // err
    return false;
  }

  auto identifier = ParseIdentifier();
  if (!identifier) {
    // err
    return false;
  }

  switch (token.type) {
  case TokenType::NEWLINE:
    top.declarations.emplace_back(
        Declaration::VariableDeclaration(*type, *identifier));
    return true;
  // Function declaration or defition
  case TokenType::LEFT_PARENTHESIS:
    return ParseFunctionDefinitionOrDeclaration(top, *type, *identifier);
    break;
  default:
    // err
    return false;
  }
}

// TODO improve error handling
Top ParseTop(Tokenizer &passedTokenizer) {
  Top top{};

  tokenizer = &passedTokenizer;

  for (;;) {
    token = tokenizer->getToken();
    tokenCategory = tokenTypeToCategory.at(token.type);
    switch (tokenCategory) {
    case TokenCategory::SEPARATOR:
      if(ParseDirective(top)){
        continue;
      }
      break;
    case TokenCategory::TYPE:
      if (ParseDeclarationOrFunction(top)) {
        continue;
      }
      break;
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