#ifndef INC_METTLE_OUTPUT_TRAITS_HPP
#define INC_METTLE_OUTPUT_TRAITS_HPP

#include <exception>
#include <iterator>
#include <ostream>
#include <type_traits>

namespace mettle {
  template<typename T> struct is_any_char : std::false_type {};
  template<typename T> struct is_any_char<const T> : is_any_char<T> {};
  template<typename T> struct is_any_char<volatile T> : is_any_char<T> {};
  template<typename T> struct is_any_char<const volatile T> : is_any_char<T> {};

  template<> struct is_any_char<char> : std::true_type {};
  template<> struct is_any_char<wchar_t> : std::true_type {};
  template<> struct is_any_char<char16_t> : std::true_type {};
  template<> struct is_any_char<char32_t> : std::true_type {};

  template<typename T>
  using is_exception = std::is_base_of<std::exception, T>;

  template<typename, typename = std::void_t<>>
  struct is_printable : std::false_type {};

  template<typename T>
  struct is_printable<T, std::void_t<
    decltype(std::declval<std::ostream&>() << std::declval<T>())
  >> : std::true_type {};

  template<typename, typename = std::void_t<>>
  struct is_iterable : std::false_type {};

  template<typename T>
  struct is_iterable<T, std::void_t<
    decltype(std::begin(std::declval<T&>()), std::end(std::declval<T&>()))
  >> : std::true_type {};


  template<typename T>
  inline constexpr bool is_any_char_v = is_any_char<T>::value;

  template<typename T>
  inline constexpr bool is_exception_v = is_exception<T>::value;

  template<typename T>
  inline constexpr bool is_iterable_v = is_iterable<T>::value;

  template<typename T>
  inline constexpr bool is_printable_v = is_printable<T>::value;

} // namespace mettle

#endif
