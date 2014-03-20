#ifndef INC_METTLE_OUTPUT_HPP
#define INC_METTLE_OUTPUT_HPP

#include <sstream>
#include <type_traits>

namespace mettle {

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
  is_printable<T>::value, decltype(std::forward<T>(t))
>::type {
  return std::forward<T>(t);
}

template<typename T>
auto ensure_printable(const T &t) -> typename std::enable_if<
  !is_printable<T>::value && !is_iterable<T>::value, std::string
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

template<typename T>
auto ensure_printable(const T &v) -> typename std::enable_if<
  !is_printable<T>::value && is_iterable<T>::value, std::string
>::type {
  std::stringstream s;
  s << "[";
  bool first = true;
  for(auto i : v) {
    if(!first)
      s << ", ";
    s << ensure_printable(i);
    first = false;
  }
  s << "]";
  return s.str();
}

} // namespace mettle

#endif
