#ifndef INC_METTLE_MATCHERS_HPP
#define INC_METTLE_MATCHERS_HPP

#include <functional>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#include "utils.hpp"

namespace mettle {

template<typename T>
class is_callable;

template<typename T, typename ...Args>
class is_callable<T(Args...)> {
  template<class U> struct always_bool { typedef bool type; };

  template<typename U, typename ...InnerArgs>
  static constexpr typename always_bool<
    decltype( std::declval<U>()(std::declval<InnerArgs>()...) )
  >::type check_(int) {
    return true;
  }
  template<typename U, typename ...InnerArgs>
  static constexpr bool check_(...) {
    return false;
  }
public:
  static const bool value = check_<T, Args...>(0);
};

template<typename T, typename Matcher>
void expect(T &&value, Matcher &&matcher) {
  auto result = matcher(value);
  if (!std::get<0>(result))
    throw expectation_error("expected " + std::get<1>(result));
}

template<typename T>
inline auto equal_to(T &&expected) {
  return [expected](auto &&actual) -> std::tuple<bool, std::string> {
    std::stringstream s;
    s << std::boolalpha << actual << " == " << expected;

    return std::make_tuple(expected == actual, s.str());
  };
}

template<typename T>
inline auto not_equal_to(T &&expected) {
  return [expected](auto &&actual) -> std::tuple<bool, std::string> {
    std::stringstream s;
    s << std::boolalpha << actual << " != " << expected;

    return std::make_tuple(expected != actual, s.str());
  };
}

template<typename T>
inline auto greater(T &&expected) {
  return [expected](auto &&actual) -> std::tuple<bool, std::string> {
    std::stringstream s;
    s << std::boolalpha << actual << " > " << expected;

    return std::make_tuple(expected > actual, s.str());
  };
}

template<typename T>
inline auto greater_equal(T &&expected) {
  return [expected](auto &&actual) -> std::tuple<bool, std::string> {
    std::stringstream s;
    s << std::boolalpha << actual << " >= " << expected;

    return std::make_tuple(expected >= actual, s.str());
  };
}

template<typename T>
inline auto less(T &&expected) {
  return [expected](auto &&actual) -> std::tuple<bool, std::string> {
    std::stringstream s;
    s << std::boolalpha << actual << " < " << expected;

    return std::make_tuple(expected < actual, s.str());
  };
}

template<typename T>
inline auto less_equal(T &&expected) {
  return [expected](auto &&actual) -> std::tuple<bool, std::string> {
    std::stringstream s;
    s << std::boolalpha << actual << " <= " << expected;

    return std::make_tuple(expected <= actual, s.str());
  };
}

template<typename T, typename U>
inline auto match(T &&matcher, U &&value, typename std::enable_if<
  is_callable<T(U)>::value
>::type* = 0) {
  return matcher(value);
}

template<typename T, typename U>
inline auto match(T &&expected, U &&actual, typename std::enable_if<
  !is_callable<T(U)>::value
>::type* = 0) {
  return equal_to(expected)(actual);
}

template<typename T>
inline auto is_not(T &&matcher) {
  return [matcher](auto &&value) -> std::tuple<bool, std::string> {
    auto result = match(matcher, value);
    return std::make_tuple(
      !std::get<0>(result), "not(" + std::get<1>(result) + ")"
    );
  };
}

template<typename ...T>
class reduce_impl {
public:
  using reducer_type = std::function<bool(bool, bool)>;
  using tuple_type = std::tuple<T...>;

  reduce_impl(const std::string &desc, const reducer_type &reducer,
              T &&...matchers)
    : desc_(desc), reducer_(reducer), matchers_(matchers...) {}

  template<typename U>
  std::tuple<bool, std::string> operator ()(U &&value) const {
    std::stringstream s;
    s << desc_ << "(";
    bool good = do_reduce<U>()(reducer_, s, matchers_, value);
    s << ")";
    return std::make_tuple(good, s.str());
  }
private:
  template<typename U, size_t N = std::tuple_size<tuple_type>::value>
  struct do_reduce {
    bool operator ()(const reducer_type &reducer, std::stringstream &s,
                     const tuple_type &tuple, U &&value) {
      constexpr auto i = std::tuple_size<tuple_type>::value - N;
      auto result = match(std::get<i>(tuple), value);
      s << std::get<1>(result) << ", ";
      return reducer(
        std::get<0>(result), do_reduce<U, N-1>()(reducer, s, tuple, value)
      );
    }
  };

  template<typename U>
  struct do_reduce<U, 1> {
    bool operator ()(const reducer_type &, std::stringstream &s,
                     const tuple_type &tuple, U &&value) {
      constexpr auto i = std::tuple_size<tuple_type>::value - 1;
      auto result = match(std::get<i>(tuple), value);
      s << std::get<1>(result);
      return std::get<0>(result);
    }
  };

  std::string desc_;
  reducer_type reducer_;
  tuple_type matchers_;
};

template<typename ...T>
inline auto any_of(T &&...matchers) {
  return reduce_impl<T...>(
    "any of", [](bool a, bool b) { return a || b; },
    std::forward<T>(matchers)...
  );
}

template<typename ...T>
inline auto all_of(T &&...matchers) {
  return reduce_impl<T...>(
    "all of", [](bool a, bool b) { return a && b; },
    std::forward<T>(matchers)...
  );
}

} // namespace mettle

#endif
