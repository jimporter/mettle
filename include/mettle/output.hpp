#ifndef INC_METTLE_OUTPUT_HPP
#define INC_METTLE_OUTPUT_HPP

#include <sstream>
#include <type_traits>
#include <typeinfo>

namespace mettle {

namespace detail {
  template<typename T>
  std::string stringify_iterable(const T &begin, const T &end);
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
constexpr auto ensure_printable(T &&t) -> typename std::enable_if<
  is_safely_printable<T>::value &&
  !std::is_array<typename std::remove_reference<T>::type>::value,
  decltype(std::forward<T>(t))
>::type {
  return std::forward<T>(t);
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

inline const char * ensure_printable(const char *s) {
  return s;
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
auto ensure_printable(T &&v) -> typename std::enable_if<
  !is_printable<T>::value && is_iterable<T>::value, std::string
>::type {
  return detail::stringify_iterable(std::begin(v), std::end(v));
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
}

} // namespace mettle

#endif
