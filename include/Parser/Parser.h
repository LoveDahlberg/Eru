
#pragma once

#include <llvm/IR/LLVMContext.h>
#include <llvm/IR/Module.h>

#include <Lexer/Lexer.h>
#include <Lexer/Tokens.h>

#include <AST/ASTContext.h>
#include <AST/Assignment.h>
#include <AST/CompilationUnit.h>
#include <AST/Controlflow.h>
#include <AST/Expression.h>
#include <AST/Types.h>

#include <Analyzer/Analyzer.h>

#include <Support/Constants.h>
#include <Support/Result.h>

using namespace Lexing;

namespace Parser {

template <typename T>
concept ValidParameterType = std::is_pointer_v<T> &&
    (std::is_same_v<AST::VariableDeclaration::Variable,
                    std::remove_pointer_t<T>> ||
     std::is_same_v<AST::Expression::Expression, std::remove_pointer_t<T>>);

class Parser {

public:
  Parser(AST::Context::ASTContext &astContext, Analyzer::Analyzer &analyzer,
         Lexer &lexer)
      : astContext(astContext), analyzer(analyzer), lexer(lexer) {}

  bool Parse();

private:
  // Support functions
  void skipUntilNotNewline();

  // Parser functions

  // Compilation unit
  bool ParseCompilationUnit();
  Result<bool> ParseVariableDeclarationOrFunction();

  // Directive
  bool ParseDirective();

  // Variable Declaration
  Result<AST::VariableDeclaration::Variable *> ParseVariable();

  // Type
  Result<AST::Types::Types> ParseType();

  // Identifier
  Result<std::string> ParseIdentifier();

  // Expression
  std::optional<AST::Expression::Expression *> ParseExpression();
  std::optional<AST::Expression::ExpressionUnit *>
  ParseExpressionUnit(bool firstUnit);

  // Assignment
  std::optional<AST::Assignment::Assignment *>
  ParseAssignment(AST::VariableDeclaration::Variable *variable = nullptr);
  std::optional<AST::Expression::Operand> ParseOperand();

  // Literal
  std::optional<std::string> ParseLiteral();

  // Statement
  std::optional<AST::Statement::Statement *> ParseStatement();

  // Controlflow
  std::optional<AST::Controlflow::ConditionalBranchingGroup *>
  ParseConditionalBranchingGroup();
  std::optional<AST::Controlflow::ConditionalBranch *>
  ParseConditionalBranch(bool start = false);

  // Function
  bool ParseFunction(AST::VariableDeclaration::Variable *variable);
  std::optional<AST::Function::Block *> ParseBlock();
  std::optional<AST::Function::FunctionCall *>
  ParseFunctionCall(std::string name);
  std::optional<AST::Function::FunctionBody *> ParseFunctionBody();

  /// This function is supposed to be used for parameter parsing for:
  /// - Function declarations and definitions -> type is
  ///   AST::VariableDeclaration::Variable*
  /// - Function calls -> type is Expression::Expression*
  ///
  ///  The \a ValidParameterType concept restricts the usage outside of these
  ///  types.
  template <typename ParameterType>
  requires ValidParameterType<ParameterType>
      std::optional<std::vector<ParameterType>> ParseParameters() {

    std::vector<ParameterType> parameters;
    if (lexer.getCurrentToken().type == TokenType::RIGHT_PARENTHESIS) {
      return parameters;
    }

    int loopCounter = 0;
    do {

      // TODO refactor this, it is not pretty.
      std::optional<ParameterType> parameterDeclaration;

      // If parameters are in a function declaration or implementation.
      if constexpr (std::is_same_v<AST::VariableDeclaration::Variable,
                                   std::remove_pointer_t<ParameterType>>) {
        parameterDeclaration = *this->ParseVariable();
      } else if constexpr (std::is_same_v<
                               AST::Expression::Expression,
                               std::remove_pointer_t<ParameterType>>) {
        parameterDeclaration = this->ParseExpression();
      } else {
        static_assert(
            always_false<ParameterType>,
            "ParameterType is not a variable nor an expression pointer");
      }

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

  AST::Context::ASTContext &astContext;
  Analyzer::Analyzer &analyzer;
  Lexer &lexer;
};

} // namespace Parser