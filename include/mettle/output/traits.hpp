#ifndef INC_METTLE_OUTPUT_TRAITS_HPP
#define INC_METTLE_OUTPUT_TRAITS_HPP

#include <exception>
#include <iterator>
#include <ostream>
#include <type_traits>

namespace mettle {
  template<typename T> constexpr bool is_character = false;
  template<typename T>
  constexpr bool is_character<const T> = is_character<T>;
  template<typename T>
  constexpr bool is_character<volatile T> = is_character<T>;
  template<typename T>
  constexpr bool is_character<const volatile T> = is_character<T>;

  template<> constexpr bool is_character<char> = true;
  template<> constexpr bool is_character<wchar_t> = true;
  template<> constexpr bool is_character<char8_t> = true;
  template<> constexpr bool is_character<char16_t> = true;
  template<> constexpr bool is_character<char32_t> = true;

  template<typename T>
  concept character = is_character<T>;

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
