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

namespace detail {
  template<typename Tuple, typename Func, typename Val,
           size_t N = std::tuple_size<Tuple>::value>
  struct do_reduce {
    auto operator ()(const Tuple &tuple, const Func &reducer,
                    Val &&value) {
      constexpr auto i = std::tuple_size<Tuple>::value - N;
      bool early_exit = false;
      auto result = reducer(
        std::forward<Val>(value), std::get<i>(tuple), early_exit
      );
      if(early_exit)
        return result;
      return do_reduce<Tuple, Func, Val, N-1>()(
        tuple, reducer, std::forward<Val>(result)
      );
    }
  };

  template<typename Tuple, typename Func, typename Val>
  struct do_reduce<Tuple, Func, Val, 1> {
    auto operator ()(const Tuple &tuple, const Func &reducer,
                    Val &&value) {
      constexpr auto i = std::tuple_size<Tuple>::value - 1;
      bool early_exit = false;
      return reducer(std::forward<Val>(value), std::get<i>(tuple), early_exit);
    }
  };

  template<typename Tuple, typename Func, typename Val>
  auto reduce_tuple(const Tuple &tuple, const Func &reducer, Val &&initial) {
    return do_reduce<Tuple, Func, Val>()(
      tuple, reducer, std::forward<Val>(initial)
    );
  }
}

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
  return {matcher, desc};
}

template<typename T, typename Matcher>
void expect(T &&value, Matcher &&matcher) {
  if (!matcher(value)) {
    std::stringstream s;
    s << std::boolalpha << "expected " << matcher.desc() << ", got " << value;
    throw expectation_error(s.str());
  }
}

template<typename T>
inline auto equal_to(T &&expected) {
  std::stringstream s;
  s << std::boolalpha << expected;
  return make_matcher([expected](auto &&actual) -> bool {
    return actual == expected;
  }, s.str());
}

template<typename T>
inline auto not_equal_to(T &&expected) {
  std::stringstream s;
  s << std::boolalpha << "not " << expected;
  return make_matcher([expected](auto &&actual) -> bool {
    return actual != expected;
  }, s.str());
}

template<typename T>
inline auto greater(T &&expected) {
  std::stringstream s;
  s << "> " << expected;
  return make_matcher([expected](auto &&actual) -> bool {
    return actual > expected;
  }, s.str());
}

template<typename T>
inline auto greater_equal(T &&expected) {
  std::stringstream s;
  s << ">= " << expected;
  return make_matcher([expected](auto &&actual) -> bool {
    return actual >= expected;
  }, s.str());
}

template<typename T>
inline auto less(T &&expected) {
  std::stringstream s;
  s << "< " << expected;
  return make_matcher([expected](auto &&actual) -> bool {
    return actual < expected;
  }, s.str());
}

template<typename T>
inline auto less_equal(T &&expected) {
  std::stringstream s;
  s << "<= " << expected;
  return make_matcher([expected](auto &&actual) -> bool {
    return actual <= expected;
  }, s.str());
}

template<typename T>
inline T& ensure_matcher(T &&matcher, typename std::enable_if<
  is_matcher<T>::value
>::type* = 0) {
  return matcher;
}

template<typename T>
inline auto ensure_matcher(T &&expected, typename std::enable_if<
  !is_matcher<T>::value
>::type* = 0) {
  return equal_to(expected);
}

template<typename T>
struct ensure_matcher_type : public std::remove_reference<
  decltype(ensure_matcher(std::declval<T>()))
> {};

template<typename T>
inline auto is_not(T &&thing) {
  auto matcher = ensure_matcher(thing);
  return make_matcher([matcher](auto &&value) -> bool {
    return !matcher(value);
  }, "not(" + matcher.desc() + ")");
}

template<typename ...T>
class reduce_impl : public matcher_tag {
public:
  using reducer_type = std::function<bool(bool, bool)>;
  using tuple_type = std::tuple<typename ensure_matcher_type<T>::type...>;

  reduce_impl(const std::string &desc, const reducer_type &reducer,
              bool initial, T &&...matchers)
    : desc_(desc), reducer_(reducer), initial_(initial),
      matchers_(ensure_matcher(matchers)...) {}

  template<typename U>
  bool operator ()(U &&value) const {
    return detail::reduce_tuple(
      matchers_, [&value, this](bool a, auto &&b, bool &early_exit) {
        bool result = reducer_(a, b(value));
        early_exit = (result != initial_);
        return result;
      }, initial_
    );
  }

  std::string desc() const {
    std::stringstream s;
    s << desc_ << "(";
    detail::reduce_tuple(matchers_, [&s](bool first, auto &&matcher, bool &) {
      if(!first)
        s << ", ";
      s << matcher.desc();
      return false;
    }, true);
    s << ")";
    return s.str();
  }
private:
  std::string desc_;
  reducer_type reducer_;
  bool initial_;
  tuple_type matchers_;
};

template<typename ...T>
inline auto any_of(T &&...matchers) {
  return reduce_impl<T...>(
    "any of", std::logical_or<bool>(), false, std::forward<T>(matchers)...
  );
}

template<typename ...T>
inline auto all_of(T &&...matchers) {
  return reduce_impl<T...>(
    "all of", std::logical_and<bool>(), true, std::forward<T>(matchers)...
  );
}

} // namespace mettle

#endif
