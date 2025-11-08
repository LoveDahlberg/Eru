
#pragma once

#include <cassert>
#include <optional>
#include <string>
#include <unordered_map>

namespace Support::Scope {

namespace {

template <typename kind>
static std::optional<kind>
getFromMap(std::unordered_map<std::string, kind> &map, const std::string name) {
  auto it = map.find(name);
  if (it == map.end()) {
    return std::nullopt;
  }
  return it->second;
}

} // namespace

enum class scopeKind {
  BASE,
  LOCAL,
  FUNCTION,
  GLOBAL,
};

template <typename variableKind> class Scope {

public:
  Scope<variableKind>(Scope<variableKind> *parentScope)
      : parentScope(parentScope) {}

  virtual scopeKind getScopeKind() { return scopeKind::BASE; }

  Scope<variableKind> *getParentScope() { return parentScope; }

  void setParentScope(Scope<variableKind> *newParentScope) {
    parentScope = newParentScope;
  }

  void addVariableDeclaration(const std::string &name,
                              const variableKind variable) {
    variableDeclarations.emplace(name, variable);
  }

  std::optional<variableKind>
  getDeclaredVariableCurrentScope(const std::string &name) {
    return getFromMap(variableDeclarations, name);
  }

  std::optional<variableKind>
  getVisibleDeclaredVariable(const std::string &name) {

    // Check declarations in current scope.
    auto currentScopeVariable = getDeclaredVariableCurrentScope(name);

    // If found in current scope, return. Otherwise check all parents for it.
    return currentScopeVariable.has_value()
               ? currentScopeVariable
               : getDeclaredVariableParentScope(name, parentScope);
  }

  std::optional<variableKind>
  getDeclaredVariableParentScope(const std::string &name,
                                 Scope<variableKind> *parentScope) {

    // If no parent, then it is never declared in parent scope.
    if (parentScope == nullptr) {
      return std::nullopt;
    }

    // Check declaration in parent.
    auto variable = getFromMap(parentScope->variableDeclarations, name);

    // If its found return it. Otherwise recursively check grandparents for it.
    return variable.has_value()
               ? variable
               : getDeclaredVariableParentScope(name, parentScope->parentScope);
  }

protected:
  // virtual ~Scope() = default;

  std::unordered_map<std::string, variableKind> variableDeclarations;
  Scope<variableKind> *parentScope;
};

template <typename variableKind, typename ContextKind>
struct LocalScope : public Scope<variableKind> {

  LocalScope(ContextKind *contextData, Scope<variableKind> *parentScope)
      : contextData(contextData), Scope<variableKind>(parentScope) {}

  ContextKind *getContextData() { return contextData; }

  virtual scopeKind getScopeKind() override { return scopeKind::LOCAL; }

protected:
  ContextKind *contextData;
};

template <typename variableKind, typename ContextKind>
struct FunctionScope : public LocalScope<variableKind, ContextKind> {

  FunctionScope<variableKind, ContextKind>(ContextKind *contextData,
                                           Scope<variableKind> *parentScope)
      : LocalScope<variableKind, ContextKind>(contextData, parentScope) {}

  scopeKind getScopeKind() override { return scopeKind::FUNCTION; }
};

template <typename variableKind, typename declarationKind>
class GlobalScope : public Scope<variableKind> {

public:
  GlobalScope<variableKind, declarationKind>() : Scope<variableKind>(nullptr) {}

  std::optional<declarationKind>
  getFunctionDeclaration(const std::string &name) {
    return getFromMap(functionDeclarations, name);
  }

  void addFunctionDeclaration(const std::string &name,
                              declarationKind &variable) {
    functionDeclarations.emplace(name, variable);
  }

  scopeKind getScopeKind() override { return scopeKind::GLOBAL; }

private:
  std::unordered_map<std::string, declarationKind> functionDeclarations;
};

template <typename variableKind, typename declarationKind, typename ContextKind>
struct ScopeHandler {

  ScopeHandler()
      : contextDataForNextLocalScope(nullptr), globalScope(),
        currentScope(&globalScope) {}

  void PrepareFunctionScope(ContextKind *contextData) {
    contextDataForNextLocalScope = contextData;
  }

  /// Call when going into a new scope
  void Push(scopeKind kind) {

    Scope<variableKind> *newChildScope = [&]() -> Scope<variableKind> * {
      switch (kind) {
      case scopeKind::BASE:
        return new Scope<variableKind>(currentScope);
      case scopeKind::LOCAL:
        return new LocalScope<variableKind, ContextKind>(
            contextDataForNextLocalScope, currentScope);
      case scopeKind::FUNCTION:
        return new FunctionScope<variableKind, ContextKind>(
            contextDataForNextLocalScope, currentScope);
      case scopeKind::GLOBAL:
        return new GlobalScope<variableKind, declarationKind>();
      }
    }();

    // Set created scope as new current scope, previous scope is set as parent.
    currentScope = newChildScope;
  }

  /// Call when going out of the current scope.
  void Pop() {
    // Nothing to pop.
    if (currentScope == nullptr || currentScope->getParentScope() == nullptr) {
      return;
    }

    switch (currentScope->getScopeKind()) {
    case scopeKind::BASE:
    case scopeKind::LOCAL:
      break;
    case scopeKind::FUNCTION:
      // End of function scope, clear context data so its not used for next
      // function.
      contextDataForNextLocalScope = nullptr;
      break;
    case scopeKind::GLOBAL:
      // Cannot pop global scope
      return;
    }

    // Drop the current scope. Set current scope to the previous parent scope.
    currentScope = currentScope->getParentScope();
  }

  LocalScope<variableKind, ContextKind> *
  CastCurrentToLocalScope() {
    if (auto *newScope =
            dynamic_cast<LocalScope<variableKind, ContextKind> *>(currentScope)) {
      return newScope;
    }
    return nullptr;
  }

  Scope<variableKind> &getCurrent() { return *currentScope; }
  GlobalScope<variableKind, declarationKind> &getGlobal() {
    return globalScope;
  }

  FunctionScope<variableKind, ContextKind> *
  findFunctionFrom(Scope<variableKind> *scope) {

    if (auto *newScope =
            dynamic_cast<FunctionScope<variableKind, ContextKind> *>(scope)) {
      return newScope;
    }

    return scope == nullptr || scope->getParentScope() == nullptr
               ? nullptr
               : findFunctionFrom(scope->getParentScope());
  }

private:
  ContextKind *contextDataForNextLocalScope;
  GlobalScope<variableKind, declarationKind> globalScope;
  std::unordered_map<std::string, variableKind> parametersForNextLocalScope;
  Scope<variableKind> *currentScope;
};

} // namespace Support::Scope
