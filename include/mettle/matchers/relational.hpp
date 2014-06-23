#ifndef INC_METTLE_MATCHERS_RELATIONAL_HPP
#define INC_METTLE_MATCHERS_RELATIONAL_HPP

#include "core.hpp"

namespace mettle {

// Predeclared in core.hpp, so we can't use deduced return types.
template<typename T>
inline auto equal_to(T &&expected) -> basic_matcher<
  std::remove_reference_t<T>, std::equal_to<>
> {
  return make_matcher(expected, std::equal_to<>(), "");
}

template<typename T>
inline auto not_equal_to(T &&expected) {
  return make_matcher(expected, std::not_equal_to<>(), "not ");
}

template<typename T>
inline auto greater(T &&expected) {
  return make_matcher(expected, std::greater<>(), "> ");
}

template<typename T>
inline auto greater_equal(T &&expected) {
  return make_matcher(expected, std::greater_equal<>(), ">= ");
}

template<typename T>
inline auto less(T &&expected) {
  return make_matcher(expected, std::less<>(), "< ");
}

template<typename T>
inline auto less_equal(T &&expected) {
  return make_matcher(expected, std::less_equal<>(), "<= ");
}

} // namespace mettle

#endif
