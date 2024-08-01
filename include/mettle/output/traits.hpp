#ifndef INC_METTLE_OUTPUT_TRAITS_HPP
#define INC_METTLE_OUTPUT_TRAITS_HPP

#include <exception>
#include <iterator>
#include <ostream>
#include <type_traits>

namespace mettle {
  template<typename T> struct is_character : std::false_type {};
  template<typename T> struct is_character<const T> : is_character<T> {};
  template<typename T> struct is_character<volatile T> : is_character<T> {};
  template<typename T> struct is_character<const volatile T>
    : is_character<T> {};

  template<> struct is_character<char> : std::true_type {};
  template<> struct is_character<wchar_t> : std::true_type {};
  template<> struct is_character<char8_t> : std::true_type {};
  template<> struct is_character<char16_t> : std::true_type {};
  template<> struct is_character<char32_t> : std::true_type {};

  template<typename T>
  concept character = is_character<T>::value;

  template<typename T>
  concept any_exception = std::derived_from<T, std::exception>;

  template<typename T>
  concept iterable = requires(T &t) {
    std::begin(t);
    std::end(t);
  };

  template<typename T>
  concept printable = requires(std::ostream &os, T &t) {
    os << t;
  };

} // namespace mettle

#endif
