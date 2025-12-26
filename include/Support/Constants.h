#pragma once

#include <cstdint>

/// An arbitrary loop limit to avoid infinite loops.
/// TODO create a infinite loop macro which always uses this limit.
constexpr const int loopLimit = 1000000;

/// An arbitrary cap of the highest number of indirection that will be
/// considered during expression parsing.
constexpr const uint32_t pointerIndirectionLimit = 0xffff;