#ifndef INC_METTLE_MATCHERS_ARITHMETIC_HPP
#define INC_METTLE_MATCHERS_ARITHMETIC_HPP

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>

#include "core.hpp"

namespace mettle {

template<typename T>
auto near_to(const T &expected, const T &epsilon) {
  std::stringstream s;
  s << "~= " << ensure_printable(expected);

  return make_matcher([expected, epsilon](const auto &actual) -> bool {
    // If one of expected or actual is NaN, mag is undefined, but that's ok
    // because we'll always return false in that case, just like we should.
    auto mag = std::max<T>(std::abs(expected), std::abs(actual));
    return std::abs(actual - expected) <= mag * epsilon;
  }, s.str());
}

template<typename T>
inline auto near_to(T &&expected) -> typename std::enable_if<
  !std::numeric_limits<T>::is_integer,
  decltype(near_to(std::declval<T>(), std::declval<T>()))
>::type {
  return near_to(std::forward<T>(expected),
                 std::numeric_limits<T>::epsilon() * 10);
}

template<typename T>
auto near_to_abs(const T &expected, const T &tolerance) {
  std::stringstream s;
  s << "~= " << ensure_printable(expected);

  return make_matcher([expected, tolerance](const auto &actual) -> bool {
    return std::abs(actual - expected) <= tolerance;
  }, s.str());
}

} // namespace mettle

#endif
