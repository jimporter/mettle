#ifndef INC_METTLE_OUTPUT_TRAITS_HPP
#define INC_METTLE_OUTPUT_TRAITS_HPP

#include <type_traits>

namespace mettle {

namespace detail {
  template<typename>
  auto check_printable(...) -> std::false_type;

  // XXX: Use std::declval here when Visual Studio gets with the program.
  template<typename T>
  auto check_printable(int) -> decltype(
    *(std::ostream*)0 << *(T*)0, std::true_type()
  );
}

template<typename T>
struct is_printable : decltype(detail::check_printable<T>(0)) {};

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

namespace detail {
  template<typename>
  auto check_iterable(...) -> std::false_type;

  // XXX: Use std::declval here when Visual Studio gets with the program.
  template<typename T>
  auto check_iterable(int) -> decltype(
    std::begin(*(T*)0), std::end(*(T*)0), std::true_type()
  );
}

template<typename T>
struct is_iterable : decltype(detail::check_iterable<T>(0)) {};

template<typename T, size_t N>
class is_iterable<T[N]> : public std::true_type {};

} // namespace mettle

#endif
