
#pragma once

#include <cstdarg>
#include <cstdio>
#include <optional>
#include <string>
#include <vector>

// TODO This does not work!

inline std::string vformat(const char *fmt, va_list args) {
  // 1) Make a true copy of the va_list
  va_list args_copy;
  va_copy(args_copy, args);

  // 2) Figure out required buffer size
  int len = std::vsnprintf(nullptr, 0, fmt, args_copy);
  va_end(args_copy); // done with the copy

  if (len < 0) {
    // formatting error
    return {};
  }

  // 3) Allocate a buffer and print into it
  std::string buf;
  buf.resize(len);
  va_list args_copy2;
  va_copy(args_copy2, args);
  std::vsnprintf(buf.data(), buf.size() + 1, fmt, args_copy2);
  va_end(args_copy2);

  return buf;
}

inline std::string format(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  std::string s = vformat(fmt, args);
  va_end(args);
  return s;
}

template <typename underLyingType> struct [[nodiscard]] Result {
  std::optional<underLyingType> value;

public:
  std::vector<std::string> failureReasons;
  bool hasFailed;

  /// Constructor to use when the operation was successful.
  /// \param value The value to use in next operation.
  Result(underLyingType value) : value(value), hasFailed(false) {}

  /// Constructor to use when the operation was a failure.
  /// \param fmtStr A format string that describes the reason for failure.
  /// \param args The argument to include in the format string, optional.
  template <typename... Args>
  Result(const char *fmtStr, Args &&...args)
      : failureReasons({format(fmtStr, std::forward<Args>(args)...)}),
        hasFailed(true) {}

  // Stores the new error message received with the old ones.
  template <typename... Args>
  void storeNewErrorMessage(const char *fmtStr, Args &&...args) {
    failureReasons.push_back(format(fmtStr, std::forward<Args>(args)...));
  }

  /// Implicit convertion operator for the new type.
  template <typename newType> operator Result<newType>() && {
    // Create the new type with the information from the old type and then
    // return it. This will be the new type used one step up in the call chain
    // hierarchy.
    return Result<newType>(hasFailed, failureReasons);
  }

  // Constructor called from the implicit converation operator, to create a new
  // object
  Result(bool hasFailed, std::vector<std::string> failureReasons)
      : hasFailed(hasFailed), failureReasons(failureReasons) {}

  underLyingType operator*() { return *value; }
};

/// Returns only when obj has failed previously. Append the formatted string to
/// the stack trace error message.
#define RET_ON_FAILURE(obj, fmt, ...)                                          \
  do {                                                                         \
    auto evaluatedObj = obj;                                                   \
    if (evaluatedObj.hasFailed) {                                              \
      evaluatedObj.storeNewErrorMessage(fmt __VA_OPT__(, ) __VA_ARGS__);       \
      return std::move(evaluatedObj);                                          \
    }                                                                          \
  } while (0)

#define RET_ON_NOT_EQUAL(left, right, fmt, ...)                                \
  do {                                                                         \
    if (left != right) {                                                       \
      return {fmt __VA_OPT__(, ) __VA_ARGS__};                                 \
    }                                                                          \
  } while (0)

#define RET_ON_EQUAL(left, right, fmt, ...)                                    \
  do {                                                                         \
    if (left == right) {                                                       \
      return {fmt __VA_OPT__(, ) __VA_ARGS__};                                 \
    }                                                                          \
  } while (0)

#define RET_ON_TRUE(boolean, fmt, ...)                                         \
  do {                                                                         \
    if (boolean) {                                                             \
      return {fmt __VA_OPT__(, ) __VA_ARGS__};                                 \
    }                                                                          \
  } while (0)

#define RET_ON_FALSE(boolean, fmt, ...)                                        \
  RET_ON_TRUE(!boolean, fmt __VA_OPT__(, ) __VA_ARGS__)
