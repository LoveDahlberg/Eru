
#include <cstdarg>
#include <cstdio>
#include <optional>
#include <string>
#include <vector>

// TODO This does not work!

static std::string vformat(const char *fmt, va_list args) {
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

static std::string format(const char *fmt, ...) {
  va_list args;
  va_start(args, fmt);
  std::string s = vformat(fmt, args);
  va_end(args);
  return s;
}

template <typename underLyingType> struct Result {
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
    if (obj.hasFailed) {                                                       \
      obj.storeNewErrorMessage(fmt __VA_OPT__(, ) __VA_ARGS__);                \
      return std::move(obj);                                                   \
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

// This code is some black magic. We call RET_ON_FAILURE with obj<firstType>
// then it returns a obj<newType> with the error message concatenated (in the
// vector).
//
// It works like this:
// 1. std::move(obj) : marks obj as an rvalue reference on the original type
//                     obj<firstType>. We do this so that only rvalue-qualified
//                     methods on Result are valid.
//
// 2. .with_context : Calls the with_context rvalue version (see the &&) on the
//                    original type obj<firstType>. This then returns the rvalue
//                    of itself std::move(*this).
//
// 3. We now are trying to return rvalue reference of type obj<firstType>, but
//    the return type is obj<newType>. Here is where the implicit conversion
//    operator 'operator Result<newType>() &&' comes in.
//
// 4. operator Result<newType>() && : The compiler now calls this implicit
//                                    conversion operator on the original type
//                                    obj<firstType>. This now creates the
//                                    Result<newType> copies over the stuff and
//                                    returns the obj<newType>.
//

// Questions: Why do we do the first std::move? Cant se just call a .convert
// function on it and then return std::move(*this)?

// TODO I simplified it, rewrite the above explaination.
