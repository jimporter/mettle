#ifndef INC_METTLE_MATCHERS_ARITHMETIC_HPP
#define INC_METTLE_MATCHERS_ARITHMETIC_HPP

#include <algorithm>
#include <cmath>
#include <cstdlib>
#include <limits>

#include "core.hpp"

namespace mettle {

  template<typename T, typename U>
  auto near_to(T &&expected, U &&epsilon) {
    return make_matcher(
      std::forward<T>(expected),
      [epsilon = std::forward<U>(epsilon)](
        const auto &actual, const auto &expected
      ) -> bool {
        // If one of expected or actual is NaN, mag is undefined, but that's ok
        // because we'll always return false in that case, just like we should.
        auto mag = std::max(std::abs(expected), std::abs(actual));
        return std::abs(actual - expected) <= mag * epsilon;
      }, "~= "
    );
  }

  template<typename T>
  inline auto near_to(T &&expected) {
    using ValueType = std::remove_cv_t<std::remove_reference_t<T>>;
    static_assert(!std::numeric_limits<ValueType>::is_integer,
                  "near_to(x) not defined for integral types");

    return near_to(std::forward<T>(expected),
                   std::numeric_limits<ValueType>::epsilon() * 10);
  }

  template<typename T, typename U>
  auto near_to_abs(T &&expected, U &&tolerance) {
    return make_matcher(
      std::forward<T>(expected),
      [tolerance = std::forward<U>(tolerance)](
        const auto &actual, const auto &expected
      ) -> bool {
        return std::abs(actual - expected) <= tolerance;
      }, "~= "
    );
  }

} // namespace mettle

#endif
