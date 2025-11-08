#include <llvm/IR/IRBuilder.h>
#include <llvm/IR/NoFolder.h>
#include <llvm/IR/Type.h>
#include <llvm/IR/Value.h>

#include <Support/Scope.h>
#include <vector>

namespace IR {

struct ScopeVariable {
  /// Creates a non pointer scope variable.
  ScopeVariable(llvm::Value *variable)
      : variable(variable), pointedToType(nullptr) {}

  /// Creates a pointer scope variable.
  ScopeVariable(llvm::Value *variable, llvm::Type *type)
      : variable(variable), pointedToType(type) {}

  /// Get the value of the variable. If it is a pointer, the value is loaded and
  /// returned.
  llvm::Value *getValue(llvm::IRBuilder<llvm::NoFolder> *builder) {
    return pointedToType == nullptr
               ? variable
               : builder->CreateLoad(pointedToType, variable);
  }

  /// If the variable is a pointer, get it. Otherwise, get the value.
  llvm::Value *getHighestOrderValue(llvm::IRBuilder<llvm::NoFolder> *builder) {
    return variable;
  }

  llvm::Value *variable;
  llvm::Type *pointedToType;
};

/// Extra information needed in the scope.
struct IRScopeContextData {
  std::unordered_map<std::string, ScopeVariable> parameters;
};

using IRScopeHandler =
    Support::Scope::ScopeHandler<ScopeVariable, llvm::Function *,
                                 IRScopeContextData>;
using IRScope = Support::Scope::Scope<ScopeVariable>;
using IRLocalScope = Support::Scope::LocalScope<ScopeVariable, IRScopeContextData>;

} // namespace IR