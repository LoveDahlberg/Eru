
// stl
#include "AST/Types.h"
#include <AST/Assignment.h>
#include <AST/Controlflow.h>
#include <AST/Declaration.h>
#include <AST/Function.h>
#include <functional>
#include <optional>
#include <string>
#include <vector>

// include
#include <AST/PrimaryExpression.h>
#include <Lexer/Tokens.h>
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

std::optional<std::string> ParseLiteral(parserItems &items) {
  if (items.lexer.getCurrentToken().type != TokenType::INTEGER_LITERAL &&
      items.lexer.getCurrentToken().type != TokenType::STRING_LITERAL) {
    // err
    return std::nullopt;
  }

  // TODO might be issues to treat ints as strings here.
  auto literal = items.lexer.getCurrentToken().value;

  // Get next, current type saved.
  items.lexer.generateNextToken();
  return literal;
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

std::optional<Declaration::VariableDeclaration *>
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

  return new Declaration::VariableDeclaration(*type, *identifier);
}

std::optional<Function::FunctionCall *>
ParseFunctionCall(parserItems &items, std::string name = "");

std::optional<Assignment::AssignmentExpressionTarget *>
ParseAssignmentExpressionTarget(parserItems &items) {
  // Determine the first target.
  switch (items.lexer.getCurrentToken().type) {

  // Identifier or function call
  case TokenType::IDENTIFER: {
    auto identifier = ParseIdentifier(items);
    if (!identifier) {
      // err
      return std::nullopt;
    }

    // Function call
    if (items.lexer.getCurrentToken().type == TokenType::LEFT_PARENTHESIS) {
      auto functionCall = ParseFunctionCall(items, *identifier);
      if (!functionCall) {
        // err
        return std::nullopt;
      }
      return new Assignment::AssignmentExpressionTarget(*functionCall);
    }
    // Identifier
    return new Assignment::AssignmentExpressionTarget(
        Types::NamedIdentifier(*identifier));
  }

  case TokenType::STRING_LITERAL: {
    auto literal = ParseLiteral(items);
    if (!literal) {
      // err
      return std::nullopt;
    }
    return new Assignment::AssignmentExpressionTarget(
        Types::StringLiteral(*literal));
  }

  case TokenType::INTEGER_LITERAL: {
    auto literal = ParseLiteral(items);
    if (!literal) {
      // err
      return std::nullopt;
    }
    return new Assignment::AssignmentExpressionTarget(
        Types::IntegerLiteral(*literal));
  }

  default: {
    break;
  }
  }
  // err
  return std::nullopt;
}

template <typename T>
concept ValidParameterType =
    std::is_pointer_v<T> &&
    (std::is_same_v<std::remove_pointer_t<T>,
                    Declaration::VariableDeclaration> ||
     std::is_same_v<std::remove_pointer_t<T>,
                    Assignment::AssignmentExpressionTarget>);

/// This function is supposed to be used for parameter parsing for:
/// - Function declarations and definitions -> type is
///   Declaration::VariableDeclaration
/// - Function calls -> type is Assignment::AssignmentExpressionTarget
///
///  The \a ValidParameterType concept restricts the usage outside of these
///  types.
///
/// \param items ..
/// \param ParseVariableType Parsing function to use for each of the two types.
///                          ParseVariableDeclaration should be used for
///                          VariableDeclaration.
///                          ParseAssignmentExpressionTarget should be used for
///                          AssignmentExpressionTarget.
template <typename ParameterType>
  requires ValidParameterType<ParameterType>
std::optional<std::vector<ParameterType>>
ParseParameters(parserItems &items,
                std::function<std::optional<ParameterType>(parserItems &)>
                    ParseVariableType) {

  std::vector<ParameterType> parameters;
  if (items.lexer.getCurrentToken().type == TokenType::RIGHT_PARENTHESIS) {
    return parameters;
  }

  int loopCounter = 0;
  do {
    auto parameterDeclaration = ParseVariableType(items);
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

std::optional<Function::FunctionCall *> ParseFunctionCall(parserItems &items,
                                                          std::string name) {

  // If name is empty, parse the identifier. Otherwise assume that it was parsed
  // before calling this function.
  if (name.empty()) {
    auto identifier = ParseIdentifier(items);
    if (!identifier) {
      // err
      return std::nullopt;
    }
    name = *identifier;
  }

  if (items.lexer.getCurrentToken().type != TokenType::LEFT_PARENTHESIS) {
    // err
    return std::nullopt;
  }

  // eat the (
  items.lexer.generateNextToken();

  auto parameters = ParseParameters<Assignment::AssignmentExpressionTarget *>(
      items, &ParseAssignmentExpressionTarget);
  if (!parameters) {
    // err
    return std::nullopt;
  }

  if (items.lexer.getCurrentToken().type != TokenType::RIGHT_PARENTHESIS) {
    // err
    return std::nullopt;
  }

  // eat the )
  items.lexer.generateNextToken();

  // TODO newline here?

  return new Function::FunctionCall(name, *parameters);
}

std::optional<Controlflow::BooleanExpression*>
ParseBooleanExpression(parserItems &items) {

  // TODO, this must be merged with AssignmentExpression.

  // do {
  //   auto target = ParseAssignmentExpressionTarget(items);
  //   if(!target)
  //   {
  //     // err
  //     return std::nullopt;
  //   }

  // }while ()
  return std::nullopt;
}

std::optional<PrimaryExpression::PrimaryExpression *>
ParsePrimaryExpression(parserItems &items);

std::optional<Controlflow::ConditionalBranch *>
ParseConditionalBranch(parserItems &items, bool start = false) {

  // TODO could refactor this to be more readable.
  if (start && items.lexer.getCurrentToken().type != TokenType::IF ||
      !start && (items.lexer.getCurrentToken().type != TokenType::ELIF ||
                 items.lexer.getCurrentToken().type == TokenType::ELSE)) {
    // err
    return std::nullopt;
  }

  bool isNotElse = items.lexer.getCurrentToken().type != TokenType::ELSE;

  // Eat the if, elif or else
  items.lexer.generateNextToken();

  auto branch = new Controlflow::ConditionalBranch();

  if (isNotElse) {

    if (items.lexer.getCurrentToken().type != TokenType::LEFT_PARENTHESIS) {
      // err
      return std::nullopt;
    }

    // Eat the (
    items.lexer.generateNextToken();

    auto booleanExpression = ParseBooleanExpression(items);
    if (!booleanExpression) {
      // err
      return std::nullopt;
    }

    if (items.lexer.getCurrentToken().type != TokenType::RIGHT_PARENTHESIS) {
      // err
      return std::nullopt;
    }
    // Eat the )
    items.lexer.generateNextToken();

    branch->addCondition(*booleanExpression);
  }

  if (items.lexer.getCurrentToken().type != TokenType::LEFT_CURLY_BRACE) {
    // err
    return std::nullopt;
  }

  // Eat the {
  items.lexer.generateNextToken();

  auto primaryExpression = ParsePrimaryExpression(items);
  if (!primaryExpression) {
    // err
    return std::nullopt;
  }

  if (items.lexer.getCurrentToken().type != TokenType::RIGHT_CURLY_BRACE) {
    // err
    return std::nullopt;
  }

  // Eat the }
  items.lexer.generateNextToken();

  branch->addPrimaryExpression(*primaryExpression);
  return branch;
}

std::optional<Controlflow::ConditionalBranchingGroup *>
ParseConditionalBranchingGroup(parserItems &items) {
  std::vector<Controlflow::ConditionalBranch *> conditionalChain;

  bool start = true;
  do {
    auto ConditionalBranch = ParseConditionalBranch(items, start);
    if (!ConditionalBranch) {
      // err
      return std::nullopt;
    }
    start = false;
    conditionalChain.push_back(*ConditionalBranch);
  } while (items.lexer.getCurrentToken().type == TokenType::ELIF ||
           items.lexer.getCurrentToken().type == TokenType::ELSE);

  return new Controlflow::ConditionalBranchingGroup(conditionalChain);
}

std::optional<Assignment::AssignmentExpression *>
ParseAssignmentExpression(parserItems &items) {

  Assignment::AssignmentExpression *expression;

  auto target = ParseAssignmentExpressionTarget(items);
  if (!target) {
    // err
    return std::nullopt;
  }
  expression->firstTarget = *target;

  // Check if assignment is over
  if (items.lexer.getCurrentToken().type == TokenType::NEWLINE) {
    expression->operation = Assignment::MathematicalOperator::END;
    return expression;
  }

  // Parse mathematical operator
  if (!Assignment::TokenToMathematicalOperator.contains(
          items.lexer.getCurrentToken().type)) {
    // err
    return std::nullopt;
  }

  expression->operation = Assignment::TokenToMathematicalOperator.at(
      items.lexer.getCurrentToken().type);

  auto secondTargetExpression = ParseAssignmentExpression(items);
  if (!secondTargetExpression) {
    // err
    return std::nullopt;
  }

  expression->SecondTarget = *secondTargetExpression;
  return expression;
}

std::optional<Assignment::Assignment *> ParseAssignment(
    parserItems &items,
    Declaration::VariableDeclaration *variableDeclaration = nullptr) {

  // If not null, variable information already parsed.
  if (variableDeclaration == nullptr) {
    // TODO implement parsing of variable declaration or single identifier
    return std::nullopt;
  }

  if (items.lexer.getCurrentToken().type != TokenType::EQUAL) {
    // err
    return std::nullopt;
  }

  // Eat the =
  items.lexer.generateNextToken();

  Assignment::Assignment *assignment;
  if (variableDeclaration->type == nullptr) {
    assignment = new Assignment::Assignment({variableDeclaration->name});
  } else {
    assignment = new Assignment::Assignment(variableDeclaration);
  }

  auto assignmentExpression = ParseAssignmentExpression(items);
  if (!assignmentExpression) {
    // err
    return std::nullopt;
  }

  assignment->setExpression(*assignmentExpression);

  return assignment;
}

std::optional<PrimaryExpression::PrimaryExpression *>
ParsePrimaryExpression(parserItems &items) {
  auto primaryExpression = new PrimaryExpression::PrimaryExpression;

  if (items.lexer.getCurrentToken().type == TokenType::RIGHT_CURLY_BRACE) {
    return primaryExpression;
  }

  do {
    auto tokenCategory =
        tokenTypeToCategory.at(items.lexer.getCurrentToken().type);
    switch (tokenCategory) {

    // Variable declaration or assignment.
    case TokenCategory::DATA_TYPE: {

      // Both start with a variable declaration.
      auto parameterDeclaration = ParseVariableDeclaration(items);
      if (!parameterDeclaration) {
        // err
        return std::nullopt;
      }

      // Assignment
      if (items.lexer.getCurrentToken().type == TokenType::EQUAL) {
        auto assignment = ParseAssignment(items, *parameterDeclaration);
        if (!assignment) {
          // err
          return std::nullopt;
        }
        primaryExpression->AddExpression(*assignment);
      }
      // Only a declaration without assignment.
      else {
        primaryExpression->AddExpression(*parameterDeclaration);
      }

      break;
    }

    // ControlFlow.
    case TokenCategory::KEYWORD: {
      // Conditional branch.
      // Always starts with an IF for now.
      if (items.lexer.getCurrentToken().type != TokenType::IF) {
        // err
        return std::nullopt;
      }

      auto controlFlow = ParseConditionalBranchingGroup(items);
      if (!controlFlow) {
        // err
        return std::nullopt;
      }

      primaryExpression->AddExpression(*controlFlow);
      break;
    }

    // Function call or assignment.
    case TokenCategory::IDENTIFER: {

      // Both start with an identifier.
      auto identifier = ParseIdentifier(items);
      if (!identifier) {
        // err
        return std::nullopt;
      }

      // TODO move this switch case to getter function.
      switch (items.lexer.getCurrentToken().type) {

      // Assignment
      case TokenType::EQUAL: {
        auto assignment = ParseAssignment(
            items, new Declaration::VariableDeclaration(nullptr, *identifier));
        if (!assignment) {
          // err
          return std::nullopt;
        }
        primaryExpression->AddExpression(*assignment);
        break;
      }

      // Function call
      case TokenType::LEFT_PARENTHESIS: {
        auto assignment = ParseFunctionCall(items, *identifier);
        if (!assignment) {
          // err
          return std::nullopt;
        }
        primaryExpression->AddExpression(*assignment);
        break;
      }

      default: {
        // err
        return std::nullopt;
      }
      }
      break;
    }
    default: {
      // err
      return std::nullopt;
    }
    }
  } while (items.lexer.generateNextToken().type !=
           TokenType::RIGHT_CURLY_BRACE);
  return primaryExpression;
}

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

  auto primaryExpression = ParsePrimaryExpression(items);
  if (!primaryExpression) {
    // err
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
  items.top.AddTopConstruct(
      new Function::FunctionDefinition(declaration, *primaryExpression));

  return true;
}

bool ParseFunctionDefinitionOrDeclaration(parserItems &items, llvm::Type *type,
                                          std::string &identifier) {
  // eat the (
  items.lexer.generateNextToken();

  auto paramaters = ParseParameters<Declaration::VariableDeclaration *>(
      items, &ParseVariableDeclaration);
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
    case TokenCategory::DATA_TYPE:
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