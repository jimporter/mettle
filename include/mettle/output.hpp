#ifndef INC_METTLE_OUTPUT_HPP
#define INC_METTLE_OUTPUT_HPP

#include <iomanip>
#include <sstream>
#include <type_traits>
#include <typeinfo>

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
  struct do_reduce<Tuple, Func, Val, 0> {
    auto operator ()(const Tuple &, const Func &, Val &&value) {
      return value;
    }
  };

  template<typename Tuple, typename Func, typename Val>
  auto reduce_tuple(const Tuple &tuple, const Func &reducer, Val &&initial) {
    return do_reduce<Tuple, Func, Val>()(
      tuple, reducer, std::forward<Val>(initial)
    );
  }

  template<typename T>
  std::string stringify_iterable(const T &begin, const T &end);

  template<typename T>
  std::string stringify_tuple(const T &tuple);
}

template<typename T>
class is_printable {
  template<typename U> struct always_bool { typedef bool type; };

  template<typename U>
  static constexpr typename always_bool<
    decltype( std::declval<std::ostream&>() << std::declval<U>() )
  >::type check_(int) {
    return true;
  }
  template<typename U>
  static constexpr bool check_(...) {
    return false;
  }
public:
  static const bool value = check_<T>(0);
};

template<typename T>
struct is_safely_printable : std::integral_constant<bool,
  is_printable<T>::value &&
  (!std::is_convertible<T, bool>::value ||
   std::is_arithmetic<typename std::remove_reference<T>::type>::value)
> {};

template<typename T>
class is_iterable {
  template<typename U> struct always_bool { typedef bool type; };

  template<typename U>
  static constexpr typename always_bool<
    decltype( std::begin(std::declval<U>()), std::end(std::declval<U>()) )
  >::type check_(int) {
    return true;
  }
  template<typename U>
  static constexpr bool check_(...) {
    return false;
  }
public:
  static const bool value = check_<T>(0);
};

template<typename T>
constexpr auto ensure_printable(const T &t) -> typename std::enable_if<
  is_safely_printable<T>::value &&
  !std::is_array<typename std::remove_reference<T>::type>::value,
  T
>::type {
  return t;
}

template<typename T>
auto ensure_printable(const T &t) -> typename std::enable_if<
  !is_safely_printable<T>::value && !is_iterable<T>::value, std::string
>::type {
  try {
    return typeid(t).name();
  }
  catch(...) {
    return "...";
  }
}

inline std::string ensure_printable(std::nullptr_t) {
  return "nullptr";
}

inline std::string ensure_printable(bool b) {
  // Yeah yeah, we could use std::boolapha here, but there's really no point.
  return b ? "true" : "false";
}

inline auto ensure_printable(const std::string &s) {
  return std::quoted(s);
}

inline auto ensure_printable(const char *s) {
  return std::quoted(s);
}

template<typename T, size_t N>
auto ensure_printable(const T (&v)[N]) -> typename std::enable_if<
  !std::is_same<
    typename std::remove_cv<typename std::make_signed<T>::type>::type,
    signed char
  >::value,
  std::string
>::type {
  return detail::stringify_iterable(std::begin(v), std::end(v));
}

template<typename T>
auto ensure_printable(const T &v) -> typename std::enable_if<
  !is_printable<T>::value && is_iterable<T>::value, std::string
>::type {
  return detail::stringify_iterable(std::begin(v), std::end(v));
}

template<typename T, typename U>
std::string ensure_printable(const std::pair<T, U> &pair) {
  return detail::stringify_tuple(pair);
}

template<typename ...T>
std::string ensure_printable(const std::tuple<T...> &tuple) {
  return detail::stringify_tuple(tuple);
}

namespace detail {
  template<typename T>
  std::string stringify_iterable(const T &begin, const T &end) {
    std::stringstream s;
    s << "[";
    if(begin != end) {
      auto i = begin;
      s << ensure_printable(*i);
      for(++i; i != end; ++i)
        s << ", " << ensure_printable(*i);
    }
    s << "]";
    return s.str();
  }

  template<typename T>
  std::string stringify_tuple(const T &tuple) {
    std::stringstream s;
    s << "[";
    reduce_tuple(tuple, [&s](bool first, const auto &x, bool &) {
      if(!first)
        s << ", ";
      s << ensure_printable(x);
      return false;
    }, true);
    s << "]";
    return s.str();
  }
}

} // namespace mettle

#endif
