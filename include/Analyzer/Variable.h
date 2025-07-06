

#include <AST/Assignment.h>
#include <Support/Result.h>

#include <memory>

namespace Analyzer {

struct Scope;
class PrivateAnalyzer;

class VariableAnalyzer {

  PrivateAnalyzer &analyzer;

  Result<bool>
  isTypeCheckedVariableDeclared(AST::VariableDeclaration::Variable *variable,
                                std::unique_ptr<Scope> &scope);
  bool isVariableDeclared(AST::VariableDeclaration::Variable *variable,
                          std::unique_ptr<Scope> &scope);

  /// Get the declared variable using the given identifers in given scope.
  AST::VariableDeclaration::Variable *
  getDeclaredVariable(AST::Types::NamedIdentifier &identifier,
                      std::unique_ptr<Scope> &scope);

  bool
  isVariableDeclaredParentScope(AST::VariableDeclaration::Variable *variable,
                                std::unique_ptr<Scope> &scope);

  AST::VariableDeclaration::Variable *
  getDeclaredVariableParentScope(AST::Types::NamedIdentifier &identifier,
                                 std::unique_ptr<Scope> &scope);

  Error addVariableDeclarationToCurrentScope(
      AST::VariableDeclaration::Variable *variable);

  Result<AST::VariableDeclaration::VariableDeclaration *>
  declareVariable(AST::VariableDeclaration::Variable *variable);

public:
  VariableAnalyzer(PrivateAnalyzer &analyser) : analyzer(analyser) {}

  /// Get the declared variable using the given identifers in current scope and
  /// all parents.
  AST::VariableDeclaration::Variable *
  getDeclaredVariable(AST::Types::NamedIdentifier &identifier);

  Error
  ActOnGlobalDeclaration(AST::VariableDeclaration::Variable *variable);
  Result<AST::VariableDeclaration::VariableDeclaration *>
  ActOnLocalDeclaration(AST::VariableDeclaration::Variable *variable);
  Error ActOnAssignment(AST::Assignment::Assignment *assignment);
};

} // namespace Analyzer