#pragma once

#include <AST/AST.h>
#include <AST/Assignment.h>
#include <AST/Controlflow.h>
#include <AST/Function.h>
#include <AST/Types.h>
#include <AST/VariableDeclaration.h>

#include <Support/Templates.h>

namespace AST::Statement {

using StatementVariant =
    std::variant<VariableDeclaration::VariableDeclaration *,
                 Assignment::Assignment *, Function::FunctionCall *,
                 Controlflow::ConditionalBranchingGroup *>;


/// Concept that the given statementType is:
/// 1. A pointer.
/// 2. One of the variants in StatementVariant.
template <typename T>
concept ValidStatementType =
    std::is_pointer_v<T> && is_one_of_variant<T, StatementVariant>::value;

struct Statement {
  template <ValidStatementType statementType>
  void AddStatement(statementType construct) {
    statements.push_back(construct);
  }

  std::vector<StatementVariant> statements;
};

} // namespace AST::Statement