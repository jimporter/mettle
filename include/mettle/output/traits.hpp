#ifndef INC_METTLE_OUTPUT_TRAITS_HPP
#define INC_METTLE_OUTPUT_TRAITS_HPP

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
struct is_boolish : std::integral_constant<bool,
  std::is_same<typename std::remove_cv<T>::type, bool>::value ||
  (std::is_convertible<T, bool>::value && !std::is_arithmetic<T>::value)
> {};

template<typename T>
struct is_boolish<T&> : is_boolish<T> {};

template<typename T>
struct is_boolish<T&&> : is_boolish<T> {};

template<typename T>
struct is_any_char_helper : std::false_type {};

template<> struct is_any_char_helper<char> : std::true_type {};
template<> struct is_any_char_helper<signed char> : std::true_type {};
template<> struct is_any_char_helper<unsigned char> : std::true_type {};

template<> struct is_any_char_helper<wchar_t> : std::true_type {};
template<> struct is_any_char_helper<char16_t> : std::true_type {};
template<> struct is_any_char_helper<char32_t> : std::true_type {};

template<typename T>
struct is_any_char : is_any_char_helper<typename std::remove_cv<T>::type> {};

template<typename T>
using is_exception = std::is_base_of<std::exception, T>;

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

template<typename T, size_t N>
class is_iterable<T[N]> : public std::true_type {};

} // namespace mettle

#endif
