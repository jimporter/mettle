#ifndef INC_METTLE_OUTPUT_TRAITS_HPP
#define INC_METTLE_OUTPUT_TRAITS_HPP

#include <cstdint>
#include <type_traits>

namespace mettle {

  namespace detail {
    template<typename T>
    struct is_any_char_helper : std::false_type {};

    template<> struct is_any_char_helper<char> : std::true_type {};
    template<> struct is_any_char_helper<signed char> : std::true_type {};
    template<> struct is_any_char_helper<unsigned char> : std::true_type {};

    template<> struct is_any_char_helper<wchar_t> : std::true_type {};
    template<> struct is_any_char_helper<char16_t> : std::true_type {};
    template<> struct is_any_char_helper<char32_t> : std::true_type {};
  }

  template<typename T>
  struct is_any_char : detail::is_any_char_helper<std::remove_cv_t<T>> {};

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
    decltype(std::begin(std::declval<T>()), std::end(std::declval<T>()))
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
