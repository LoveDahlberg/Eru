
// stl
#include <optional>
#include <string>
#include <vector>

// include
#include "AST/Declaration.h"
#include "Lexer/Tokens.h"
#include <Parser/Parser.h>
#include <Support/Constants.h>
#include <Support/Log.h>

// llvm
#include "llvm/IR/Type.h"
#include <llvm/Support/Error.h>

namespace Parser {

// TODO move this to a printing file
constexpr auto topParsingName = "TopParsing";
constexpr auto declarationParsingName = "DeclarationParsing";
constexpr auto directiveParsingName = "DirectiveParsing";
constexpr auto typeParsingName = "TypeParsing";

void printParsing(const char *parsingFunctionName, TokenCategory tokenCategory,
                  Token token) {
  // LogError("{}: Unexpected token of:\nCategory '{}'\n Type '{}'\n "
  //          "Value '{}'\n",
  //          parsingFunctionName,
  //          std::to_string(static_cast<int>(tokenCategory)),
  //          std::to_string(static_cast<int>(token.type)), token.value);
}

void skipUntilNotNewline(parserItems &items) {
  if (items.lexer.getCurrentToken().type == Lexing::TokenType::NEWLINE) {
    return;
  }

  while (items.lexer.generateNextToken().type != Lexing::TokenType::NEWLINE) {
  }
}

bool ParseDirective(parserItems &items) {
  if (items.lexer.getCurrentToken().type != TokenType::LEFT_BRACKET) {
    // err
    return false;
  }

  // Eat [
  items.lexer.generateNextToken();

  if (items.lexer.getCurrentToken().type != TokenType::RIGHT_BRACKET) {
    // err
    return false;
  }

  // Eat ]
  items.lexer.generateNextToken();

  return true;
}

std::optional<std::string> ParseIdentifier(parserItems &items) {
  if (items.lexer.getCurrentToken().type != TokenType::IDENTIFER) {
    // err
    return std::nullopt;
  }

  auto identifier = items.lexer.getCurrentToken().value;

  // Get next, current type saved.
  items.lexer.generateNextToken();
  return identifier;
}

std::optional<llvm::Type *> ParseType(parserItems &items) {

  llvm::Type *type;

  switch (items.lexer.getCurrentToken().type) {
  case TokenType::INT:
    type = llvm::Type::getInt32Ty(items.module->getContext());
    break;
  case TokenType::SIGNED_INT_32:
    type = llvm::Type::getInt32Ty(items.module->getContext());
    break;
  case TokenType::UNSIGNED_INT_32:
    type = llvm::Type::getInt32Ty(items.module->getContext());
    break;
  case TokenType::BOOl:
    type = llvm::Type::getInt1Ty(items.module->getContext());
    break;
  case TokenType::CHAR:
    type = llvm::Type::getInt8Ty(items.module->getContext());
    break;
  case TokenType::STRING:
    // TODO implement string handling
    type = llvm::StructType::create(items.module->getContext(), "string");
    break;
  default:
    // err
    return std::nullopt;
  }

  // Get next, current type saved.
  items.lexer.generateNextToken();
  return type;
}

std::optional<Declaration::VariableDeclaration>
ParseVariableDeclaration(parserItems &items) {
  auto type = ParseType(items);
  if (!type) {
    // err
    return std::nullopt;
  }

  auto identifier = ParseIdentifier(items);
  if (!identifier) {
    // err
    return std::nullopt;
  }

  return Declaration::VariableDeclaration(*type, *identifier);
}

std::optional<std::vector<Declaration::VariableDeclaration>>
ParseParameters(parserItems &items) {

  std::vector<Declaration::VariableDeclaration> parameters;
  if (items.lexer.getCurrentToken().type == TokenType::RIGHT_PARENTHESIS) {
    return parameters;
  }

  int loopCounter = 0;
  do {
    auto parameterDeclaration = ParseVariableDeclaration(items);
    if (!parameterDeclaration) {
      // err
      return std::nullopt;
    }
    parameters.push_back(*parameterDeclaration);

    if (items.lexer.getCurrentToken().type == TokenType::RIGHT_PARENTHESIS) {
      break;
    }

    if (items.lexer.getCurrentToken().type != TokenType::COMMA) {
      // err
      return std::nullopt;
    }

    // Eat the ,
    items.lexer.generateNextToken();
  } while (loopCounter++ < loopLimit);

  return parameters;
}

bool ParsePrimaryExpression() { return true; }

bool ParseFunctionDefinition(parserItems &items,
                             Declaration::FunctionDeclaration *declaration) {

  auto directive = ParseDirective(items);

  skipUntilNotNewline(items);

  if (items.lexer.getCurrentToken().type != TokenType::LEFT_CURLY_BRACE) {
    // err
    return false;
  }

  // eat the {
  items.lexer.generateNextToken();

  if (!ParsePrimaryExpression()) {
    return false;
  }

  skipUntilNotNewline(items);

  if (items.lexer.getCurrentToken().type != TokenType::RIGHT_CURLY_BRACE) {
    // err
    return false;
  }

  // eat the }
  items.lexer.generateNextToken();

  // Create the function based on the declaration and the primary expression.

  // items.top.functions.

  return true;
}

bool ParseFunctionDefinitionOrDeclaration(parserItems &items, llvm::Type *type,
                                          std::string &identifier) {
  // eat the (
  items.lexer.generateNextToken();

  auto paramaters = ParseParameters(items);
  if (!paramaters) {
    // err
    return false;
  }

  // TODO strictly not needed, as a valid ParseParameters will leave the
  // parenthesis here. But its more correct to check it here too.
  if (items.lexer.getCurrentToken().type != TokenType::RIGHT_PARENTHESIS) {
    // err
    return false;
  }

  // eat the )
  items.lexer.generateNextToken();

  skipUntilNotNewline(items);

  auto declaration =
      new Declaration::FunctionDeclaration(type, identifier, *paramaters);

  if (items.lexer.getCurrentToken().type == TokenType::LEFT_BRACKET) {
    return ParseFunctionDefinition(items, declaration);
  }

  items.top.AddTopConstruct(declaration);
  return true;
}

bool ParseDeclarationOrFunction(parserItems &items) {
  auto type = ParseType(items);
  if (!type) {
    // err
    return false;
  }

  auto identifier = ParseIdentifier(items);
  if (!identifier) {
    // err
    return false;
  }

  switch (items.lexer.getCurrentToken().type) {
  case TokenType::NEWLINE:
    items.top.AddTopConstruct(
        new Declaration::VariableDeclaration(*type, *identifier));
    return true;
  // Function declaration or defition
  case TokenType::LEFT_PARENTHESIS:
    return ParseFunctionDefinitionOrDeclaration(items, *type, *identifier);
    break;
  default:
    // err
    return false;
  }
}

// TODO improve error handling
parserItems ParseTop(Lexer &lexer) {
  parserItems items(lexer);

  int loopCounter = 0;
  do {
    auto tokenCategory =
        tokenTypeToCategory.at(items.lexer.generateNextToken().type);
    switch (tokenCategory) {
    case TokenCategory::SEPARATOR:
      if (items.lexer.getCurrentToken().type != TokenType::LEFT_BRACKET) {
        continue;
      }

      if (ParseDirective(items)) {
        continue;
      }
      break;
    case TokenCategory::TYPE:
      if (ParseDeclarationOrFunction(items)) {
        continue;
      }
      break;
    default:
      if (items.lexer.getCurrentToken().type == TokenType::END_OF_FILE) {
        break;
      }
      // err
      printParsing(topParsingName, tokenCategory,
                   items.lexer.getCurrentToken());
      break;
    }
    // Break the main loop
    break;
  } while (loopCounter++ < loopLimit);

  return items;
}

} // namespace Parser