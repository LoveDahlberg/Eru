
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

#define RET_ON_WRONG_TOKEN(expectedTokenType, fmt, ...)                        \
  RET_ON_NOT_EQUAL(lexer.getCurrentToken(), expectedTokenType,            \
                   fmt __VA_OPT__(, ) __VA_ARGS__)

template <typename T>
concept ValidParameterType = std::is_pointer_v<T> &&
    (std::is_same_v<AST::VariableDeclaration::Variable,
                    std::remove_pointer_t<T>> ||
     std::is_same_v<AST::Expression::Expression, std::remove_pointer_t<T>>);

struct FunctionBodyToParse {
  indexType startIndex;
  AST::Function::Function* function;
};
class Parser {

public:
  Parser(AST::Context::ASTContext &astContext, Analyzer::PublicAnalyzer &analyzer,
         Lexer &lexer)
      : astContext(astContext), analyzer(analyzer), lexer(lexer) {}

  Result<bool> Parse();

private:
  // Support functions
  void skipUntilNotNewline();

  // Parser functions

  // Compilation unit
  Result<bool> ParseTopLevelItems();
  Result<bool> ParseFunctionBodies();
  Result<bool> ParseVariableDeclarationOrFunction();

  // Directive
  Result<bool> ParseDirective();

  // Variable Declaration
  Result<AST::VariableDeclaration::Variable *> ParseVariable();

  // Type
  Result<AST::Types::Types> ParseType();

  // Identifier
  Result<std::string> ParseIdentifier();

  // Expression
  Result<AST::Expression::Expression *> ParseExpression(AST::Types::Types expectedType = AST::Types::NONE);
  Result<AST::Expression::ExpressionUnit *> ParseExpressionUnit(bool firstUnit);

  // Assignment
  Result<AST::Assignment::Assignment *>
  ParseAssignment(AST::VariableDeclaration::Variable *variable = nullptr);
  Result<AST::Expression::Operand> ParseOperand();

  // Literal
  Result<std::string> ParseLiteral();

  // Statement
  Result<AST::Statement::Statement *> ParseStatement();

  // Controlflow
  Result<AST::Controlflow::ConditionalBranchingGroup *>
  ParseConditionalBranchingGroup();
  Result<AST::Controlflow::ConditionalBranch *>
  ParseConditionalBranch(bool start = false);

  // Function
  Result<bool> ParseFunction(AST::VariableDeclaration::Variable *variable);
  Result<AST::Function::Block *> ParseBlock();
  Result<AST::Function::FunctionCall *> ParseFunctionCall(std::string name);
  Result<AST::Function::FunctionBody *> ParseFunctionBody();
  Result<bool> SkipFunctionBody();

  /// This function is supposed to be used for parameter parsing for:
  /// - Function declarations and definitions -> type is
  ///   AST::VariableDeclaration::Variable*
  /// - Function calls -> type is Expression::Expression*
  ///
  ///  The \a ValidParameterType concept restricts the usage outside of these
  ///  types.
  template <typename ParameterType>
  requires ValidParameterType<ParameterType> Result<std::vector<ParameterType>>
  ParseParameters() {

    std::vector<ParameterType> parameters;
    if (lexer.getCurrentToken() == TokenType::RIGHT_PARENTHESIS) {
      return parameters;
    }

    int loopCounter = 0;
    do {

      // TODO refactor this, it is not pretty.

      // If parameters are in a function declaration or implementation.
      if constexpr (std::is_same_v<AST::VariableDeclaration::Variable,
                                   std::remove_pointer_t<ParameterType>>) {
        auto variable = this->ParseVariable();

        RET_ON_FAILURE(variable, "ParseParameters: Failed variable.");

        parameters.push_back(*variable);
      } else if constexpr (std::is_same_v<
                               AST::Expression::Expression,
                               std::remove_pointer_t<ParameterType>>) {
        auto expression = this->ParseExpression();

        RET_ON_FAILURE(expression, "ParseParameters: Failed expression.");

        parameters.push_back(*expression);
      } else {
        static_assert(
            always_false<ParameterType>,
            "ParameterType is not a variable nor an expression pointer");
      }

      if (lexer.getCurrentToken() == TokenType::RIGHT_PARENTHESIS) {
        break;
      }

      RET_ON_WRONG_TOKEN(TokenType::COMMA,
                         "ParseParameters: Failed, missing comma.");

      // Eat the ,
      lexer.generateNextToken();
    } while (loopCounter++ < loopLimit);

    return parameters;
  }

  AST::Context::ASTContext &astContext;
  Analyzer::PublicAnalyzer &analyzer;
  Lexer &lexer;

  std::vector<FunctionBodyToParse> functionBodiesToParse;
};

} // namespace Parser