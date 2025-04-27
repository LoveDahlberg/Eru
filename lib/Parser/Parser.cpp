
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
#include <AST/Statement.h>
#include <Lexer/Tokens.h>
#include <Parser/Parser.h>
#include <Support/Constants.h>
#include <Support/Log.h>

// llvm
#include "llvm/IR/Type.h"
#include <llvm/Support/Error.h>

namespace Parser {

// TODO move this to a printing file
constexpr auto CompilationUnitParsingName = "CompilationUnitParsing";
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

std::optional<Expression::Operand> ParseOperand(parserItems &items) {
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
      return Expression::Operand(*functionCall);
    }
    // Identifier
    return Expression::Operand(Types::NamedIdentifier(*identifier));
  }

  case TokenType::STRING_LITERAL: {
    auto literal = ParseLiteral(items);
    if (!literal) {
      // err
      return std::nullopt;
    }
    return Expression::Operand(Types::StringLiteral(*literal));
  }

  case TokenType::INTEGER_LITERAL: {
    auto literal = ParseLiteral(items);
    if (!literal) {
      // err
      return std::nullopt;
    }
    return Expression::Operand(Types::IntegerLiteral(*literal));
  }

  default: {
    break;
  }
  }
  // err
  return std::nullopt;
}

std::optional<Expression::ExpressionUnit *>
ParseExpressionUnit(parserItems &items) {

  auto operand = ParseOperand(items);
  if (!operand) {
    // err
    return std::nullopt;
  }

  auto unit = new Expression::ExpressionUnit();
  unit->operand = *operand;

  // TODO create proper operator map and category.
  switch (items.lexer.getCurrentToken().type) {
  case TokenType::AND:
  case TokenType::OR: {
    unit->operation = Expression::TokenToBooleanOperator.at(
        items.lexer.getCurrentToken().type);
    items.lexer.generateNextToken();
    break;
  }

  case TokenType::PLUS:
  case TokenType::MINUS: {
    unit->operation = Expression::TokenToArithmeticOperator.at(
        items.lexer.getCurrentToken().type);
    items.lexer.generateNextToken();
    break;
  }

  default: {
    break;
  }
  }

  return unit;
}

template <typename T>
concept ValidParameterType =
    std::is_pointer_v<T> &&
    (std::is_same_v<std::remove_pointer_t<T>,
                    Declaration::VariableDeclaration> ||
     std::is_same_v<std::remove_pointer_t<T>, Expression::ExpressionUnit>);

/// This function is supposed to be used for parameter parsing for:
/// - Function declarations and definitions -> type is
///   Declaration::VariableDeclaration
/// - Function calls -> type is Expression::ExpressionUnit
///
///  The \a ValidParameterType concept restricts the usage outside of these
///  types.
///
/// \param items ..
/// \param ParseVariableType Parsing function to use for each of the two types.
///                          ParseVariableDeclaration should be used for
///                          VariableDeclaration.
///                          ParseAssignmentExpressionTarget should be used for
///                          Expression::ExpressionUnit.
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

  auto parameters = ParseParameters<Expression::ExpressionUnit *>(
      items, &ParseExpressionUnit);
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

std::optional<Expression::Expression *> ParseExpression(parserItems &items) {

  Expression::Expression *expression;

  int loopCounter = 0;
  do {
    auto target = ParseExpressionUnit(items);
    if (!target) {
      // err
      return std::nullopt;
    }

    expression->addExpressionUnit(*target);

    // Expression is over if no extra operand was parsed at the end.
    if ((*target)->operation == std::nullopt) {
      break;
    }
  } while (loopCounter++ < loopLimit);

  return expression;
}

std::optional<Statement::Statement *> ParseStatement(parserItems &items);

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

    auto expression = ParseExpression(items);
    if (!expression) {
      // err
      return std::nullopt;
    }

    if (items.lexer.getCurrentToken().type != TokenType::RIGHT_PARENTHESIS) {
      // err
      return std::nullopt;
    }
    // Eat the )
    items.lexer.generateNextToken();

    branch->addExpression(*expression);
  }

  if (items.lexer.getCurrentToken().type != TokenType::LEFT_CURLY_BRACE) {
    // err
    return std::nullopt;
  }

  // Eat the {
  items.lexer.generateNextToken();

  auto statement = ParseStatement(items);
  if (!statement) {
    // err
    return std::nullopt;
  }

  if (items.lexer.getCurrentToken().type != TokenType::RIGHT_CURLY_BRACE) {
    // err
    return std::nullopt;
  }

  // Eat the }
  items.lexer.generateNextToken();

  branch->addStatement(*statement);
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

  auto expression = ParseExpression(items);
  if (!expression) {
    // err
    return std::nullopt;
  }

  assignment->setExpression(*expression);

  if (items.lexer.getCurrentToken().type != TokenType::NEWLINE) {
    // err
    return std::nullopt;
  }

  return assignment;
}

std::optional<Statement::Statement *> ParseStatement(parserItems &items) {
  auto statement = new Statement::Statement;

  if (items.lexer.getCurrentToken().type == TokenType::RIGHT_CURLY_BRACE) {
    return statement;
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
        statement->AddStatement(*assignment);
      }
      // Only a declaration without assignment.
      else {
        statement->AddStatement(*parameterDeclaration);
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

      statement->AddStatement(*controlFlow);
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
        statement->AddStatement(*assignment);
        break;
      }

      // Function call
      case TokenType::LEFT_PARENTHESIS: {
        auto assignment = ParseFunctionCall(items, *identifier);
        if (!assignment) {
          // err
          return std::nullopt;
        }
        statement->AddStatement(*assignment);
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
  return statement;
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

  auto statement = ParseStatement(items);
  if (!statement) {
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
  items.compilationUnit.AddCompilationUnitItems(
      new Function::FunctionDefinition(declaration, *statement));

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

  items.compilationUnit.AddCompilationUnitItems(declaration);
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
    items.compilationUnit.AddCompilationUnitItems(
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
parserItems ParseCompilationUnit(Lexer &lexer) {
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
      printParsing(CompilationUnitParsingName, tokenCategory,
                   items.lexer.getCurrentToken());
      break;
    }
    // Break the main loop
    break;
  } while (loopCounter++ < loopLimit);

  return items;
}

} // namespace Parser