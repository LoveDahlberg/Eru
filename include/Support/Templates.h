#pragma once

#include <variant>

/// Compile time type predicate for checking if a template is a std::variant.
/// This default predicate applies to any T doesn't have a partial specialization.
/// In other words, all types except for std::variant will be false.
template <typename T> struct is_variant : std::false_type {};

/// A partial specialization of is_variant. This makes so that 
/// std::variant returns true when this predicate is used.
template <typename... Ts>
struct is_variant<std::variant<Ts...>> : std::true_type {};

/// Check if template is a std::variant.
template <typename T>
concept IsVariant = is_variant<T>::value;

/// The primary template declaration of is_one_of_variant. It is needed for the
/// below specialization to be implemented.
template <typename T, typename Variant> struct is_one_of_variant;

/// A partial specialization of is_one_of_variant. It is used to
/// statically verify that T is the same as one of the items in the
/// given std::variant.
///
/// The std::disjunction inheritance essentially makes so that is_one_of_variant
/// inherits from the result of std::is_same<T, U1>::value || std::is_same<T,
/// U2>::value || ...
template <typename T, typename... Us>
struct is_one_of_variant<T, std::variant<Us...>>
    : std::disjunction<std::is_same<T, Us>...> {};