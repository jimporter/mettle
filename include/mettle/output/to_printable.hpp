#ifndef INC_METTLE_OUTPUT_TO_PRINTABLE_HPP
#define INC_METTLE_OUTPUT_TO_PRINTABLE_HPP

#include <cstddef>
#include <cstdint>
#include <cstdlib>
#include <locale>
#include <iomanip>
#include <sstream>
#include <string>
#include <string_view>

#include "string.hpp"
#include "traits.hpp"
#include "type_name.hpp"
#include "../detail/algorithm.hpp"
#include "../detail/tuple_algorithm.hpp"

namespace mettle {

  template<typename T>
  std::string to_printable(T begin, T end);

  // Non-generic overloads

  inline std::string to_printable(std::nullptr_t) {
    return "nullptr";
  }

  template<typename Char, typename Traits, typename Alloc>
  inline std::string to_printable(
    const std::basic_string<Char, Traits, Alloc> &s
  ) requires string_convertible<std::basic_string<Char, Traits, Alloc>> {
    return represent_string(s);
  }

  template<typename Char, typename Traits>
  inline std::string to_printable(
    const std::basic_string_view<Char, Traits> &s
  ) requires string_convertible<std::basic_string_view<Char, Traits>> {
    return represent_string(s);
  }

  inline std::string to_printable(char c) {
    return represent_string(std::string(1, c), '\'');
  }

  inline std::string to_printable(wchar_t c) {
    return represent_string(std::wstring(1, c), '\'');
  }

  inline std::string to_printable(char8_t c) {
    return represent_string(std::u8string(1, c), '\'');
  }

  inline std::string to_printable(char16_t c) {
    return represent_string(std::u16string(1, c), '\'');
  }

  inline std::string to_printable(char32_t c) {
    return represent_string(std::u32string(1, c), '\'');
  }

  inline std::string to_printable(unsigned char c) {
    std::ostringstream ss;
    ss << "0x" << std::setw(2) << std::setfill('0') << std::hex
       << static_cast<unsigned int>(c);
    return ss.str();
  }

  inline std::string to_printable(signed char c) {
    std::ostringstream ss;
    ss << (c >= 0 ? '+' : '-') << "0x" << std::setw(2) << std::setfill('0')
       << std::hex << std::abs(static_cast<int>(c));
    return ss.str();
  }

  inline std::string to_printable(std::byte b) {
    return to_printable(static_cast<unsigned char>(b));
  }

  // Pointers

  template<typename T>
  // Prevent array->pointer decay here.
  std::string to_printable(const T &t) requires(std::is_pointer_v<T>) {
    if(!t) return to_printable(nullptr);

    using ValueType = const std::remove_pointer_t<T>;
    std::ostringstream ss;
    if constexpr(character<ValueType>) {
      return represent_string(t);
    } else {
      std::ostringstream ss;
      if constexpr(std::same_as<ValueType, const unsigned char> ||
                   std::same_as<ValueType, const signed char>) {
        // Don't print signed/unsigned char* as regular strings.
        ss << static_cast<const void *>(t);
      } else {
        ss << t;
      }
      return ss.str();
    }
  }

  template<typename Ret, typename ...Args>
  inline auto to_printable(Ret (*)(Args...)) {
    return type_name<Ret(Args...)>();
  }

  // Other scalars

  template<typename T>
  std::string to_printable(const T &t) requires(std::is_enum_v<T>) {
    return type_name<T>() + "(" + std::to_string(
      static_cast<std::underlying_type_t<T>>(t)
    ) + ")";
  }

  template<typename T>
  auto to_printable(const T &t) requires(std::same_as<T, bool>) {
    return t ? "true" : "false";
  }

  // Collections

  namespace detail {
    template<typename T>
    std::string stringify_tuple(const T &tuple);
  }

  template<typename T, typename U>
  std::string to_printable(const std::pair<T, U> &pair) {
    return detail::stringify_tuple(pair);
  }

  template<typename ...T>
  std::string to_printable(const std::tuple<T...> &tuple) {
    return detail::stringify_tuple(tuple);
  }

  template<character Char, std::size_t N>
  std::string to_printable(const Char (&s)[N]) {
    return to_printable(static_cast<const Char *>(s));
  }

  template<typename T, std::size_t N>
  std::string to_printable(const T (&t)[N]) {
    return to_printable(std::begin(t), std::end(t));
  }

  // Fallback implementation. Try to print various types in a reasonable way.

  template<typename T>
  inline auto to_printable(const T &t) {
    if constexpr(printable<T>) {
      return t;
    } else if constexpr(any_exception<T>) {
      std::ostringstream ss;
      ss << type_name(t) << "(" << to_printable(t.what()) << ")";
      return ss.str();
    } else if constexpr(iterable<T>) {
      return to_printable(std::begin(t), std::end(t));
    } else {
      return type_name<T>();
    }
  }

  // These need to be last in order for the `to_printable()` calls inside to
  // pick up all the above implementations (plus any others via ADL).

  template<typename T>
  inline auto to_printable(const volatile T &t) {
    return to_printable(const_cast<const T &>(t));
  }

  template<typename T>
  std::string to_printable(T begin, T end) {
    std::ostringstream ss;
    ss << "[" << detail::iter_joined(begin, end, [](auto &&item) {
      return to_printable(item);
    }) << "]";
    return ss.str();
  }

  namespace detail {
    template<typename T>
    std::string stringify_tuple(const T &tuple) {
      std::ostringstream ss;
      ss << "[" << tuple_joined(tuple, [](auto &&item) {
        return to_printable(item);
      }) << "]";
      return ss.str();
    }
  }

} // namespace mettle

#endif
