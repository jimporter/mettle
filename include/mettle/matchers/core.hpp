#ifndef INC_METTLE_MATCHERS_CORE_HPP
#define INC_METTLE_MATCHERS_CORE_HPP

#include <sstream>
#include <string>
#include <type_traits>
#include <utility>

#include "../output.hpp"
#include "any_capture.hpp"
#include "result.hpp"

namespace mettle {

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
    return to_printable(std::forward<T>(expected));
  }
}

template<typename T, typename F>
class basic_matcher : public matcher_tag {
public:
  template<typename T2, typename F2>
  basic_matcher(T2 &&thing, F2 &&f, std::string prefix)
    : thing_(std::forward<T2>(thing)), f_(std::forward<F2>(f)),
      prefix_(std::move(prefix)) {}

  template<typename U>
  auto operator ()(U &&actual) const {
    return f_(std::forward<U>(actual), thing_.value);
  }

  std::string desc() const {
    std::ostringstream ss;
    ss << prefix_ << detail::matcher_desc(thing_.value);
    return ss.str();
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
  basic_matcher(F2 &&f, std::string desc)
    : f_(std::forward<F2>(f)), desc_(std::move(desc)) {}

  template<typename U>
  auto operator ()(U &&actual) const {
    return f_(std::forward<U>(actual));
  }

  const std::string & desc() const {
    return desc_;
  }
private:
  F f_;
  std::string desc_;
};

template<typename T, typename F, typename String>
inline auto make_matcher(T &&thing, F &&f, String &&prefix) {
  return basic_matcher<
    std::remove_reference_t<T>, std::remove_reference_t<F>
  >(std::forward<T>(thing), std::forward<F>(f), std::forward<String>(prefix));
}

template<typename F, typename String>
inline auto make_matcher(F &&f, String &&desc) {
  return basic_matcher<
    void, std::remove_reference_t<F>
  >(std::forward<F>(f), std::forward<String>(desc));
}

template<typename T>
auto equal_to(T &&expected) {
  return make_matcher(std::forward<T>(expected), std::equal_to<>(), "");
}

inline auto anything() {
  return make_matcher([](const auto &) -> bool {
    return true;
  }, "anything");
}

template<typename T>
inline auto ensure_matcher(T &&matcher) -> typename std::enable_if<
  is_matcher<T>::value, decltype(std::forward<T>(matcher))
>::type {
  return std::forward<T>(matcher);
}

template<typename T>
inline auto ensure_matcher(T &&expected) -> typename std::enable_if<
  !is_matcher<T>::value, decltype( equal_to(std::forward<T>(expected)) )
>::type {
  return equal_to(std::forward<T>(expected));
}

template<typename T>
struct ensure_matcher_type : public std::remove_reference<
  decltype(ensure_matcher(std::declval<T>()))
> {};

template<typename T>
using ensure_matcher_t = typename ensure_matcher_type<T>::type;

template<typename T>
inline auto is_not(T &&thing) {
  return make_matcher(
    ensure_matcher(std::forward<T>(thing)),
    [](const auto &value, auto &&matcher) {
      return !matcher(value);
    }, "not "
  );
}

template<typename T>
inline auto describe(T &&matcher, const std::string &desc) {
  return make_matcher(std::forward<T>(matcher), desc);
}

template<typename Filter, typename Matcher>
auto filter(Filter &&f, Matcher &&matcher, const std::string &desc = "") {
  return make_matcher(
    std::forward<Matcher>(matcher),
    [f = std::forward<Filter>(f), desc](const auto &actual, auto &&matcher) {
      auto filtered = f(actual);
      return match_result(
        matcher(filtered), desc + detail::stringify(to_printable(filtered))
      );
    }, desc
  );
}

} // namespace mettle

#endif
