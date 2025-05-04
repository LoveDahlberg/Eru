#pragma once

#include <AST/AST.h>
#include <AST/Assignment.h>
#include <AST/Controlflow.h>
#include <AST/VariableDeclaration.h>
#include <AST/Function.h>
#include <AST/Types.h>

namespace AST::Statement {

// TODO I prefer not doing it this way, ideally we just use inheritence, but
// that becomes a bit annyoing with circual references.
template <typename T>
concept ValidstatementType =
    std::is_pointer_v<T> &&
    (std::is_same_v<VariableDeclaration::VariableDeclaration,
                    std::remove_pointer_t<T>> ||
     std::is_same_v<Assignment::Assignment, std::remove_pointer_t<T>> ||
     std::is_same_v<Function::FunctionCall, std::remove_pointer_t<T>> ||
     std::is_base_of_v<Controlflow::Controlflow, std::remove_pointer_t<T>>);

struct Statement : public AST {
  template <typename statementType>
    requires ValidstatementType<statementType>
  void AddStatement(statementType construct) {
    statements.push_back(construct);
  }

  llvm::Value * codegen(llvm::Module &module) override;

private:
  std::vector<AST *> statements;
};

} // namespace AST::Statement