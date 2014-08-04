#ifndef INC_METTLE_MATCHERS_CORE_HPP
#define INC_METTLE_MATCHERS_CORE_HPP

#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#include "output.hpp"
#include "error.hpp"
#include "any_capture.hpp"

namespace mettle {

// TODO: There's probably a better way to ensure something is a matcher, but
// this'll do for now.
struct matcher_tag {};

template<typename T>
struct is_matcher : public std::is_base_of<
  matcher_tag, typename std::remove_reference<T>::type
> {};

namespace detail {
  template<typename T>
  inline auto matcher_desc(T &&matcher, typename std::enable_if<
    is_matcher<T>::value
  >::type* = 0) {
    return std::forward<T>(matcher).desc();
  }

  template<typename T>
  inline auto matcher_desc(T &&expected, typename std::enable_if<
    !is_matcher<T>::value
  >::type* = 0) {
    return ensure_printable(std::forward<T>(expected));
  }
}

template<typename T, typename F>
class basic_matcher : public matcher_tag {
public:
  template<typename T2, typename F2>
  basic_matcher(T2 &&thing, F2 &&f, const std::string &prefix)
    : thing_(std::forward<T2>(thing)), f_(std::forward<F2>(f)),
      prefix_(prefix) {}

  template<typename U>
  bool operator ()(U &&actual) const {
    return f_(std::forward<U>(actual), thing_.value);
  }

  std::string desc() const {
    std::stringstream s;
    s << prefix_ << detail::matcher_desc(thing_.value);
    return s.str();
  }
private:
  any_capture<T> thing_;
  F f_;
  std::string prefix_;
};

template<typename F>
class basic_matcher<void, F> : public matcher_tag {
public:
  template<typename F2>
  basic_matcher(F2 &&f, const std::string &desc)
    : f_(std::forward<F2>(f)), desc_(desc) {}

  template<typename U>
  bool operator ()(U &&actual) const {
    return f_(std::forward<U>(actual));
  }

  const std::string & desc() const {
    return desc_;
  }
private:
  F f_;
  std::string desc_;
};

template<typename T, typename F>
inline auto make_matcher(T &&thing, F &&f, const std::string &prefix) {
  return basic_matcher<
    std::remove_reference_t<T>, std::remove_reference_t<F>
  >(std::forward<T>(thing), std::forward<F>(f), prefix);
}

template<typename F>
inline auto make_matcher(F &&f, const std::string &desc) {
  return basic_matcher<
    void, std::remove_reference_t<F>
  >(std::forward<F>(f), desc);
}

template<typename T>
auto equal_to(T &&expected) {
  return make_matcher(std::forward<T>(expected), std::equal_to<>(), "");
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
  return make_matcher(
    ensure_matcher(std::forward<T>(thing)),
    [](const auto &value, auto &&matcher) -> bool {
      return !matcher(value);
    }, "not "
  );
}

template<typename T>
inline auto describe(T &&matcher, const std::string &desc) {
  return make_matcher(std::forward<T>(matcher), desc);
}

} // namespace mettle

#endif
