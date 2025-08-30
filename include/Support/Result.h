
#pragma once

#include <cassert>
#include <optional>
#include <sstream>
#include <string>
#include <vector>

/// This file essentially reimplements llvm::Expected<> and llvm::Error.
/// Difference is that this uses my custom formatter, stores a string based call
/// chain with formatted descriptions and stores the code snippet of when the
/// problem occured.

class Formatter {
  std::string result;

public:
  Formatter() : result({}){};

  template <typename... Args> Formatter(Args &&...args) {
    std::ostringstream stream;

    // fold‑expression: stream << arg1 << arg2 << ... << argN;
    (stream << ... << std::forward<Args>(args));

    result = stream.str();
  }

  operator std::string() const & { return result; }
  operator std::string() && { return std::move(result); }
  const std::string &get() const noexcept { return result; }
};

struct Void {
  Void() = delete;
};

// TODO remove the explicit boolean from functions that actually dont return
// anything.
template <typename underLyingType = Void> struct [[nodiscard]] Result {
  std::optional<underLyingType> value;
  bool checked = false;

public:
  std::vector<std::string> failureDescription;
  std::string codeSnippet{};
  bool hasFailed;

  bool hasSucceded() { return !hasFailed; }

  ~Result() {
    if (!checked) {
      assert("Discarding a Result object without checking its status");
    }
  }

  /// Constructor to use when the operation was successful and the value should
  /// be propagated.
  /// \param value The value to use in next operation.
  Result(underLyingType value) requires(
      !std::is_same_v<underLyingType, Formatter>)
      : value(value), hasFailed(false) {}

  /// Constructor to use when the operation was successful and no value needs to
  /// be propagated.
  Result() requires(std::is_same_v<underLyingType, Void>) : hasFailed(false) {}

  /// Constructor to use when the operation was a failure.
  /// \param format A format string that describes the reason for failure.
  Result(const Formatter description, const Formatter code = {})
      : failureDescription({description}), codeSnippet(code), hasFailed(true) {
    // auto codeString = code.get();
    // if (!codeString.empty()) {
    //   codeSnippet = codeString);
    // }
  }

  /// Checks if an error has occoured. If it has, store the incoming description
  /// and code and return false. Otherwise return true.
  bool check(const Formatter description, const Formatter code = {}) {
    checked = true;
    if (hasFailed) {
      storeNewStackTrace(description, code);
      return false;
    }
    return true;
  }

  bool check()
  {
    checked = true;
    if (hasFailed) {
      return false;
    }
    return true;
  }

  // Stores the new error message received with the old ones.
  void storeNewStackTrace(const Formatter description,
                          const Formatter code = {}) {
    failureDescription.push_back(description);

    // Only add code snippet if its not already there.
    auto codeString = code.get();
    if (codeSnippet.empty()) {
      codeSnippet = codeString;
    }
  }

  /// Implicit convertion operator for the new type.
  template <typename newType> operator Result<newType>() && {
    // Create the new type with the information from the old type and then
    // return it. This will be the new type used one step up in the call chain
    // hierarchy.
    return Result<newType>(hasFailed, failureDescription, codeSnippet);
  }

  /// Get the underlying value. Only enable this for non-void values.
  underLyingType operator*() requires(!std::is_same_v<underLyingType, Void>) {
    if (hasFailed) {
      assert("Cannot get resulting value when an error has occoured.");
      return {};
    }
    return *value;
  }

  // Constructor called from the implicit converation operator, to create a new
  // object
  Result(bool hasFailed, std::vector<std::string> failureDescription,
         std::string codeSnippet)
      : hasFailed(hasFailed), failureDescription(failureDescription),
        codeSnippet(codeSnippet) {}
};

using Error = Result<>;

#define SUCCESS Result<>()

#define FAILURE(description)                                                   \
  Formatter { description }
#define FAILURE_CODE(description, lexer)                                       \
  { Formatter{description}, lexer.getParsedInput() }

// Anon namespace to not expose internal macros.
namespace {

/// Returns only when obj has failed previously. Append the error description
/// and optional a code snippet.
#define RET_ON_FAILURE_FULL(obj, desc, code)                                   \
  do {                                                                         \
    auto evaluatedObj = obj;                                                   \
    if (!evaluatedObj.check(desc, code)) {                                     \
      return std::move(evaluatedObj);                                          \
    }                                                                          \
  } while (0)

#define RET_ON_(what, desc, code)                                              \
  do {                                                                         \
    if (what) {                                                                \
      return {desc, code};                                                     \
    }                                                                          \
  } while (0)

} // namespace

// Variants for return on without code snippet.

#define RET_ON_FAILURE(obj, desc) RET_ON_FAILURE_FULL(obj, desc, {""})

#define RET_ON_NOT_EQUAL(left, right, desc)                                    \
  RET_ON_((left) != (right), desc, {""})

#define RET_ON_EQUAL(left, right, desc) RET_ON_((left) == (right), desc, {""})

#define RET_ON_TRUE(boolean, desc) RET_ON_((boolean), desc, {""})

#define RET_ON_FALSE(boolean, desc) RET_ON_(!(boolean), desc, {""})

// Variants for return on with code snippet.

#define RET_ON_FAILURE_CODE(obj, desc, lexer)                                  \
  RET_ON_FAILURE_FULL(obj, desc, lexer.getParsedInput())

#define RET_ON_NOT_EQUAL_CODE(left, right, desc, lexer)                        \
  RET_ON_((left) != (right), desc, lexer.getParsedInput())

#define RET_ON_EQUAL_CODE(left, right, desc, lexer)                            \
  RET_ON_((left) == (right), desc, lexer.getParsedInput())

#define RET_ON_TRUE_CODE(boolean, desc, lexer)                                 \
  RET_ON_((boolean), desc, lexer.getParsedInput())

#define RET_ON_FALSE_CODE(boolean, desc, lexer)                                \
  RET_ON_(!(boolean), desc, lexer.getParsedInput())
