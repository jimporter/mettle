#ifndef INC_METTLE_MATCHERS_RELATIONAL_HPP
#define INC_METTLE_MATCHERS_RELATIONAL_HPP

#include "core.hpp"

namespace mettle {

  // Note: equal_to is declared in core.hpp, since it's pretty important!

  template<typename T>
  inline auto not_equal_to(T &&expected) {
    return make_matcher(std::forward<T>(expected), std::not_equal_to<>(),
                        "not ");
  }

  template<typename T>
  inline auto greater(T &&expected) {
    return make_matcher(std::forward<T>(expected), std::greater<>(), "> ");
  }

  template<typename T>
  inline auto greater_equal(T &&expected) {
    return make_matcher(std::forward<T>(expected), std::greater_equal<>(),
                        ">= ");
  }

  template<typename T>
  inline auto less(T &&expected) {
    return make_matcher(std::forward<T>(expected), std::less<>(), "< ");
  }

  template<typename T>
  inline auto less_equal(T &&expected) {
    return make_matcher(std::forward<T>(expected), std::less_equal<>(), "<= ");
  }

} // namespace mettle

#endif
