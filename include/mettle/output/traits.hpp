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

  namespace detail {
    // Make a special kind of ostream that can't print types that would use the
    // boolean overload.
    struct strict_ostream : std::ostream {};
    inline void operator <<(strict_ostream &, bool) {}

    template<typename T>
    struct ostream_for { using type = std::ostream; };
    template<typename T> requires std::is_class_v<T>
    struct ostream_for<T> { using type = strict_ostream; };

    template<typename T>
    using ostream_for_t = typename ostream_for<T>::type;
  }

  template<typename T>
  concept printable = requires(detail::ostream_for_t<T> &os, T &t) {
    { os << t } -> std::convertible_to<std::ostream&>;
  };

} // namespace mettle

#endif
