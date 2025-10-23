
#pragma once

#include <cassert>
#include <memory>
#include <optional>
#include <unordered_map>
#include <vector>

namespace Support::Scope {

template <typename variableKind> class Scope {

public:
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

  std::optional<variableKind> getDeclaredVariable(const std::string &name) {

    // Check declarations in current scope.
    auto currentScopeVariable = getDeclaredVariableCurrentScope(name);

    // If found in current scope, return. Otherwise check all parents for it.
    return currentScopeVariable.has_value()
               ? currentScopeVariable
               : getDeclaredVariableParentScope(name, parentScope);
  }

  virtual bool isGlobal() const { return false; }

protected:
  virtual ~Scope() = default;

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

  template <typename kind>
  static std::optional<kind>
  getFromMap(std::unordered_map<std::string, kind> &map,
             const std::string name) {
    auto it = map.find(name);
    if (it == map.end()) {
      return std::nullopt;
    }
    return it->second;
  }

  std::unordered_map<std::string, variableKind> variableDeclarations;
  Scope<variableKind> *parentScope;
};

template <typename variableKind>
struct LocalScope : public Scope<variableKind> {

  LocalScope(const std::unordered_map<std::string, variableKind> &parameters) {
    Scope<variableKind>::variableDeclarations.insert(parameters.begin(),
                                                     parameters.end());
  }
};

template <typename variableKind, typename functionKind>
class GlobalScope : public Scope<variableKind> {

public:
  // TODO: These are all functions, not only declarations (declarations are only
  // externs techincally).
  std::optional<functionKind> getFunctionDeclaration(const std::string &name) {
    return Scope<variableKind>::getFromMap(functionDeclarations, name);
  }

  void addFunctionDeclaration(const std::string &name, functionKind &variable) {
    functionDeclarations.emplace(name, variable);
  }

  bool isGlobal() const override { return true; }

private:
  std::unordered_map<std::string, functionKind> functionDeclarations;
};

template <typename variableKind, typename functionKind> struct ScopeHandler {

  ScopeHandler() : globalScope(), currentScope(&globalScope) {}

  Scope<variableKind> &getCurrent() { return *currentScope; }
  GlobalScope<variableKind, functionKind> &getGlobal() { return globalScope; }

  /// Adds to the parameters list for the next local scope that is pushed. It
  /// solves the issue of not having access to the parameters when the actual
  /// scope is pushed.
  void AddParametersForNextPushedLocalScope(const std::string &name,
                                            const variableKind &variable) {
    if (!parametersForNextLocalScope.contains(name)) {
      parametersForNextLocalScope.emplace(name, variable);
    }
  }

  /// Call when going into a new scope
  void Push() {

    // New scope is always a local scope.
    Scope<variableKind> *newChildScope =
        new LocalScope<variableKind>(parametersForNextLocalScope);

    parametersForNextLocalScope.clear();

    newChildScope->setParentScope(std::move(currentScope));
    currentScope = std::move(newChildScope);
  }

  /// Call when going out of the current scope.
  void Pop() {
    // Nothing to pop.
    if (currentScope == nullptr || currentScope->isGlobal() ||
        currentScope->getParentScope() == nullptr) {
      return;
    }

    // Drop the current scope. Set current scope to the previous parent scope.
    currentScope = std::move(currentScope->getParentScope());
  }

private:
  std::unordered_map<std::string, variableKind> parametersForNextLocalScope;

  GlobalScope<variableKind, functionKind> globalScope;
  Scope<variableKind> *currentScope;
};

} // namespace Support::Scope
