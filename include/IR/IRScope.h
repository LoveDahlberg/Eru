#include <AST/Types.h>
#include <Support/Scope.h>

#include "ScopeVariable.h"
namespace IR {

/// Extra information gathered at function declaration. This is then parsed
/// and stored during function definition.
struct ScopeFunction {
  ScopeFunction() : function(nullptr), underlyingParameterTypes() {}

  llvm::Function *function;

  std::vector<AST::Types::Type> underlyingParameterTypes;
};

/// Extra information needed in the scope.
struct IRScopeContextData {
  std::unordered_map<std::string, ScopeVariable> parameters;
  llvm::Function *currentFunction = nullptr;
};

using IRScopeHandler =
    Support::Scope::ScopeHandler<ScopeVariable *, ScopeFunction,
                                 IRScopeContextData>;
using IRScope = Support::Scope::Scope<ScopeVariable *>;
using IRLocalScope =
    Support::Scope::LocalScope<ScopeVariable *, IRScopeContextData>;

} // namespace IR