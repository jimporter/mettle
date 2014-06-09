#ifndef INC_METTLE_MATCHERS_CORE_HPP
#define INC_METTLE_MATCHERS_CORE_HPP

#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#include "../output.hpp"
#include "../error.hpp"

namespace mettle {

// TODO: There's probably a better way to ensure something is a matcher, but
// this'll do for now.
struct matcher_tag {};

template<typename T>
struct is_matcher : public std::is_base_of<
  matcher_tag, typename std::remove_reference<T>::type
> {};

template<typename T>
class basic_matcher : public matcher_tag {
public:
  using function_type = T;

  basic_matcher(const function_type &function, const std::string &desc)
    : f_(function), desc_(desc) {}

  template<typename U>
  auto operator ()(U &&actual) const {
    return f_(std::forward<U>(actual));
  }

  const std::string & desc() const {
    return desc_;
  }
private:
  function_type f_;
  std::string desc_;
};

template<typename T>
basic_matcher<T> make_matcher(T &&matcher, const std::string &desc) {
  return {std::forward<T>(matcher), desc};
}

template<typename T, typename Matcher>
void expect(const T &value, const Matcher &matcher) {
  if(!matcher(value)) {
    std::stringstream s;
    s << "expected " << matcher.desc() << ", got " << ensure_printable(value);
    throw expectation_error(s.str());
  }
}

inline auto anything() {
  return make_matcher([](const auto &) -> bool {
    return true;
  }, "anything");
}

template<typename T>
inline auto equal_to(const T &expected) {
  std::stringstream s;
  s << ensure_printable(expected);
  return make_matcher([expected](const auto &actual) -> bool {
    return actual == expected;
  }, s.str());
}

template<typename T>
inline auto ensure_matcher(T &&matcher, typename std::enable_if<
  is_matcher<T>::value
>::type* = 0) {
  return std::forward<T>(matcher);
}

template<typename T>
inline auto ensure_matcher(T &&expected, typename std::enable_if<
  !is_matcher<T>::value
>::type* = 0) {
  return equal_to(std::forward<T>(expected));
}

template<typename T>
struct ensure_matcher_type : public std::remove_reference<
  decltype(ensure_matcher(std::declval<T>()))
> {};

template<typename T>
inline auto is_not(T &&thing) {
  auto matcher = ensure_matcher(std::forward<T>(thing));
  return make_matcher([matcher](const auto &value) -> bool {
    return !matcher(value);
  }, "not " + matcher.desc());
}

} // namespace mettle

#endif
