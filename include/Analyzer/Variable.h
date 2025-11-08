

#include <AST/Assignment.h>
#include <Support/Result.h>
#include <Analyzer/AnalyzerTypes.h>

#include <memory>

using namespace Support::Scope;

namespace Analyzer {

class PrivateAnalyzer;

class VariableAnalyzer {

  PrivateAnalyzer &analyzer;

  Result<bool>
  isTypeCheckedVariableDeclared(AST::VariableDeclaration::Variable *variable,
                                bool checkParents);
  bool isVariableDeclaredGlobally(AST::VariableDeclaration::Variable *variable);

  bool
  isVariableDeclaredParentScope(AST::VariableDeclaration::Variable *variable,
                                AnalyzerScope &scope);

  AST::VariableDeclaration::Variable *
  getDeclaredVariableParentScope(AST::Types::NamedIdentifier &identifier,
                                 AnalyzerScope &scope);

  Error addVariableDeclarationInCurrentScope(
      AST::VariableDeclaration::Variable *variable);

  Result<AST::VariableDeclaration::VariableDeclaration *>
  declareVariable(AST::VariableDeclaration::Variable *variable);

public:
  VariableAnalyzer(PrivateAnalyzer &analyser) : analyzer(analyser) {}

  /// Get the declared variable using the given identifers in current scope and
  /// all parents.
  AST::VariableDeclaration::Variable *
  getDeclaredVariable(AST::Types::NamedIdentifier &identifier);

  Error ActOnGlobalDeclaration(
      AST::VariableDeclaration::Variable *variable,
      std::optional<AST::Expression::ConstantOperand> constOperand);
  Result<AST::VariableDeclaration::VariableDeclaration *>
  ActOnLocalDeclaration(AST::VariableDeclaration::Variable *variable);
  Error ActOnAssignment(AST::Assignment::Assignment *assignment);
};
} // namespace Analyzer