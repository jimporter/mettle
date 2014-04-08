#ifndef INC_METTLE_MATCHERS_BOOLEAN_HPP
#define INC_METTLE_MATCHERS_BOOLEAN_HPP

#include "core.hpp"

namespace mettle {

// Note: equal_to is in core.hpp, since it's pretty important!

template<typename T>
inline auto not_equal_to(const T &expected) {
  std::stringstream s;
  s << "not " << ensure_printable(expected);
  return make_matcher([expected](const auto &actual) -> bool {
    return actual != expected;
  }, s.str());
}

template<typename T>
inline auto greater(const T &expected) {
  std::stringstream s;
  s << "> " << ensure_printable(expected);
  return make_matcher([expected](const auto &actual) -> bool {
    return actual > expected;
  }, s.str());
}

template<typename T>
inline auto greater_equal(const T &expected) {
  std::stringstream s;
  s << ">= " << ensure_printable(expected);
  return make_matcher([expected](const auto &actual) -> bool {
    return actual >= expected;
  }, s.str());
}

template<typename T>
inline auto less(const T &expected) {
  std::stringstream s;
  s << "< " << ensure_printable(expected);
  return make_matcher([expected](const auto &actual) -> bool {
    return actual < expected;
  }, s.str());
}

template<typename T>
inline auto less_equal(const T &expected) {
  std::stringstream s;
  s << "<= " << ensure_printable(expected);
  return make_matcher([expected](const auto &actual) -> bool {
    return actual <= expected;
  }, s.str());
}

} // namespace mettle

#endif
