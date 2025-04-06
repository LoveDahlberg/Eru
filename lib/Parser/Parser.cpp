
// stl
#include "AST/Types.h"
#include "Lexer/Tokens.h"
#include <optional>
#include <string>
#include <vector>

// include
#include <Parser/Parser.h>
#include <Support/Constants.h>
#include <Support/Log.h>

// llvm
#include <llvm/Support/Error.h>

namespace Parser {

// TODO move this to a printing file
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

bool ParseDirective(Top &top) { return false; }

std::optional<Types::Identifier> ParseIdentifier(Lexer &lexer) {
  if (lexer.getCurrentToken().type != TokenType::IDENTIFER) {
    // err
    return std::nullopt;
  }

  auto identifier = Types::Identifier(lexer.getCurrentToken().value);

  // Get next, current type saved.
  lexer.generateNextToken();
  return identifier;
}

std::optional<Types::Type> ParseType(Lexer &lexer) {

  Types::Type type;

  switch (lexer.getCurrentToken().type) {
  case TokenType::INT:
    type = Types::Int();
    break;
  case TokenType::IDENTIFER:
    type = Types::Identifier(lexer.getCurrentToken().value);
    break;
  case TokenType::SIGNED_INT_32:
    type = Types::SingedInt32();
    break;
  case TokenType::UNSIGNED_INT_32:
    type = Types::UnsignedInt32();
    break;
  case TokenType::BOOl:
    type = Types::Bool();
    break;
  case TokenType::STRING:
    type = Types::String();
    break;
  case TokenType::CHAR:
    type = Types::Char();
    break;
  default:
    // err
    return std::nullopt;
  }

  // Get next, current type saved.
  lexer.generateNextToken();
  return type;
}

std::optional<Declaration::Declaration> ParseVariableDeclaration(Lexer &lexer) {
  auto type = ParseType(lexer);
  if (!type) {
    // err
    return std::nullopt;
  }

  auto identifier = ParseIdentifier(lexer);
  if (!identifier) {
    // err
    return std::nullopt;
  }

  return Declaration::VariableDeclaration(*type, *identifier);
}

std::optional<std::vector<Declaration::Declaration>>
ParseParameters(Lexer &lexer) {

  std::vector<Declaration::Declaration> parameters;
  if (lexer.getCurrentToken().type == TokenType::RIGHT_PARENTHESIS) {
    return parameters;
  }

  int loopCounter = 0;
  do {
    auto parameterDeclaration = ParseVariableDeclaration(lexer);
    if (!parameterDeclaration) {
      // err
      return std::nullopt;
    }
    parameters.push_back(*parameterDeclaration);

    if (lexer.getCurrentToken().type == TokenType::RIGHT_PARENTHESIS) {
      break;
    }

    if (lexer.getCurrentToken().type != TokenType::COMMA) {
      // err
      return std::nullopt;
    }

    // Eat the ,
    lexer.generateNextToken();
  } while (loopCounter++ < loopLimit);

  return parameters;
}

bool ParseFunctionDefinition(Lexer &lexer, Top &top, const Types::Type &type,
                             const Types::Identifier &identifier,
                             std::vector<Declaration::Declaration> parameters) {
  return true;
}

bool ParseFunctionDefinitionOrDeclaration(Lexer &lexer, Top &top,
                                          const Types::Type &type,
                                          const Types::Identifier &identifier) {
  // eat the (
  lexer.generateNextToken();

  auto paramaters = ParseParameters(lexer);
  if (!paramaters) {
    // err
    return false;
  }

  // TODO strictly not needed, as a valid ParseParameters will leave the
  // parenthesis here. But its more correct to check it here too.
  if (lexer.getCurrentToken().type != TokenType::RIGHT_PARENTHESIS) {
    // err
    return false;
  }

  // eat the )
  lexer.generateNextToken();

  if (lexer.getCurrentToken().type == TokenType::NEWLINE) {
    top.declarations.emplace_back(
        Declaration::FunctionDeclaration(type, identifier, *paramaters));
    return true;
  }

  return ParseFunctionDefinition(lexer, top, type, identifier, *paramaters);
}

bool ParseDeclarationOrFunction(Lexer &lexer, Top &top) {
  auto type = ParseType(lexer);
  if (!type) {
    // err
    return false;
  }

  auto identifier = ParseIdentifier(lexer);
  if (!identifier) {
    // err
    return false;
  }

  switch (lexer.getCurrentToken().type) {
  case TokenType::NEWLINE:
    top.declarations.emplace_back(
        Declaration::VariableDeclaration(*type, *identifier));
    return true;
  // Function declaration or defition
  case TokenType::LEFT_PARENTHESIS:
    return ParseFunctionDefinitionOrDeclaration(lexer, top, *type, *identifier);
    break;
  default:
    // err
    return false;
  }
}

// TODO improve error handling
Top ParseTop(Lexer &lexer) {
  Top top{};

  int loopCounter = 0;
  do {
    auto tokenCategory = tokenTypeToCategory.at(lexer.generateNextToken().type);
    switch (tokenCategory) {
    case TokenCategory::SEPARATOR:
      if (lexer.getCurrentToken().type != TokenType::LEFT_BRACKET) {
        continue;
      }

      if (ParseDirective(top)) {
        continue;
      }
      break;
    case TokenCategory::TYPE:
      if (ParseDeclarationOrFunction(lexer, top)) {
        continue;
      }
      break;
    default:
      if (lexer.getCurrentToken().type == TokenType::END_OF_FILE) {
        break;
      }
      // err
      printParsing(topParsingName, tokenCategory, lexer.getCurrentToken());
      break;
    }
    // Break the main loop
    break;
  } while (loopCounter++ < loopLimit);

  return top;
}

} // namespace Parser