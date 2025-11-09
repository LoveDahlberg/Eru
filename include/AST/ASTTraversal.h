#include <AST/Function.h>
#include <Support/Result.h>
#include <AST/ASTContext.h>
#include <AST/CompilationUnit.h>
#include <AST/Statement.h>
#include <AST/VariableDeclaration.h>

// stl
#include <vector>

using namespace AST;

/// Base class for walking the AST.
template <typename ReturnType> struct ASTTraversal {

  // Walk the given AST.
  std::vector<Result<ReturnType>> Walk(Context::ASTContext &Context) {
    return handleAll(Context.getAST()->compilationUnitItems);
  }

  std::vector<Result<ReturnType>> handle(Statement::Statement &statement) {
    return handleAll(statement.statements);
  }

  /// Entrypoint for handling of each type.

  virtual Result<ReturnType> handle(Assignment::Assignment &AST) = 0;
  virtual Result<ReturnType> handle(Controlflow::ConditionalBranchingGroup &AST) = 0;
  virtual Result<ReturnType> handle(Expression::Expression &AST) = 0;
  virtual Result<ReturnType> handle(VariableDeclaration::VariableDeclaration &AST) = 0;
  virtual Result<ReturnType> handle(Function::FunctionCall &AST) = 0;
  virtual Result<ReturnType> handle(Function::Block &AST) = 0;
  virtual Result<ReturnType> handle(Function::FunctionBody &AST) = 0;
  virtual Result<ReturnType> handle(Function::FunctionDeclaration &AST) = 0;
  virtual Result<ReturnType>
  handle(VariableDeclaration::GlobalVariableInitialization &AST) = 0;

private:
  /// Helper function that calls handle on all std::variant items in a vector.
  /// It applies std::visit on each variant passed and calls the correct
  /// handle overload.
  ///
  /// It returns the result from each handle call as a vector.
  ///
  /// TODO could consider typechecking the types of the variant, unless
  /// the error message of the std::visit is clear enough (likely not).
  template <IsVariant type>
  std::vector<Result<ReturnType>> handleAll(std::vector<type> itemsToVisit) {

    std::vector<Result<ReturnType>> result;
    result.reserve(itemsToVisit.size());

    for (auto item : itemsToVisit) {
      auto r = std::visit(
          [this](auto *nodePtr) -> Result<ReturnType> {
            return this->handle(*nodePtr);
          },
          item);
      result.push_back(std::move(r));
    }

    return result;
  }
};
