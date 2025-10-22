
#pragma once

#include <cassert>
#include <memory>
#include <unordered_map>
#include <vector>

namespace Support::Scope {

namespace {

template <typename kind>
kind getFromMap(std::unordered_map<std::string, kind> &map,
                const std::string name) {
  auto it = map.find(name);
  if (it == map.end()) {
    return nullptr;
  }
  return it->second;
}

} // namespace

template <typename variableKind> struct Scope {
  virtual ~Scope() = default;

  variableKind getDeclaredVariable(const std::string &name) {
    return getFromMap(variableDeclarations, name);
  }

  std::unordered_map<std::string, variableKind> variableDeclarations;

  virtual bool isGlobal() const { return false; }
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
struct GlobalScope : public Scope<variableKind> {

  // TODO: These are all functions, not only declarations (declarations are only
  // externs techincally).
  functionKind getFunctionDeclaration(const std::string &name) {
    return getFromMap(functionDeclarations, name);
  }

  std::unordered_map<std::string, functionKind> functionDeclarations;

  bool isGlobal() const override { return true; }
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

    newChildScope->parentScope = std::move(currentScope);
    currentScope = std::move(newChildScope);
  }

  /// Call when going out of the current scope.
  void Pop() {
    // Nothing to pop.
    if (currentScope == nullptr || currentScope->isGlobal() ||
        currentScope->parentScope == nullptr) {
      return;
    }

    // Drop the current scope. Set current scope to the previous parent scope.
    currentScope = std::move(currentScope->parentScope);
  }

private:
  std::unordered_map<std::string, variableKind> parametersForNextLocalScope;

  GlobalScope<variableKind, functionKind> globalScope;
  Scope<variableKind> *currentScope;
};

} // namespace Support::Scope
