#pragma once

#include <Parser/Syntax/Syntax.h>
#include <Parser/Syntax/VariableDeclaration.h>

using namespace ::AST::Function;
using functionAST = ::AST::Function::Function;

namespace Parser::Syntax::Function {
bool ParseFunction(syntaxItems &items, Variable *declaration);

std::optional<FunctionCall *> ParseFunctionCall(syntaxItems &items,
                                                std::string name);

template <typename T>
concept ValidParameterType =
    std::is_pointer_v<T> &&
    (std::is_same_v<Variable, std::remove_pointer_t<T>> ||
     std::is_same_v<::AST::Expression::Expression, std::remove_pointer_t<T>>);

/// This function is supposed to be used for parameter parsing for:
/// - Function declarations and definitions -> type is
///   AST::VariableDeclaration::Variable
/// - Function calls -> type is Expression::Expression
///
///  The \a ValidParameterType concept restricts the usage outside of these
///  types.
///
/// \param items ..
/// \param ParseVariableType Parsing function to use for each of the two types.
///                          Variable should be used for
///                          VariableDeclaration.
///                          ParseAssignmentExpressionTarget should be used for
///                          Expression::Expression.
template <typename ParameterType>
  requires ValidParameterType<ParameterType>
std::optional<std::vector<ParameterType>>
ParseParameters(syntaxItems &items,
                std::function<std::optional<ParameterType>(syntaxItems &)>
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
} // namespace Parser::Syntax::Function