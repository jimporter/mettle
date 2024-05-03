#ifndef INC_METTLE_OUTPUT_TO_PRINTABLE_HPP
#define INC_METTLE_OUTPUT_TO_PRINTABLE_HPP

#include <cstdint>
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
  inline std::string
  to_printable(const std::basic_string<Char, Traits, Alloc> &s) {
    return escape_string(string_convert(s));
  }

  template<typename Char, typename Traits>
  inline std::string
  to_printable(const std::basic_string_view<Char, Traits> &s) {
    return escape_string(string_convert(s));
  }

  inline std::string to_printable(char c) {
    return escape_string(std::string(1, c), '\'');
  }

  inline std::string to_printable(unsigned char c) {
    return escape_string(std::string(1, c), '\'');
  }

  inline std::string to_printable(signed char c) {
    return escape_string(std::string(1, c), '\'');
  }

  inline std::string to_printable(wchar_t c) {
    return escape_string(string_convert(std::wstring(1, c)), '\'');
  }

  inline std::string to_printable(char16_t c) {
    return escape_string(string_convert(std::u16string(1, c)), '\'');
  }

  inline std::string to_printable(char32_t c) {
    return escape_string(string_convert(std::u32string(1, c)), '\'');
  }

  template<typename T>
  inline auto to_printable(const T *s) -> std::enable_if_t<
    is_any_char_v<T>, std::string
  > {
    if(!s) return to_printable(nullptr);
    return escape_string(string_convert(s));
  }

  template<typename Ret, typename ...Args>
  inline auto to_printable(Ret (*)(Args...)) {
    return type_name<Ret(Args...)>();
  }

  // Containers

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

  template<typename T, std::size_t N>
  auto to_printable(const T (&v)[N]) -> std::enable_if_t<
    !is_any_char_v<T>, std::string
  > {
    return to_printable(std::begin(v), std::end(v));
  }

  // The main `to_printable` implementation. This needs to be after the
  // non-generic versions above so that those overloads get picked up by the
  // calls inside this function.

  template<typename T>
  inline auto to_printable(const T &t) {
    if constexpr(std::is_pointer_v<T>) {
      using ValueType = std::remove_pointer_t<T>;
      if constexpr(!std::is_const_v<ValueType>) {
        return to_printable(const_cast<const ValueType*>(t));
      } else {
        if(!t) return to_printable(nullptr);
        std::ostringstream ss;
        ss << t;
        return ss.str();
      }
    } else if constexpr(std::is_enum_v<T>) {
      return type_name<T>() + "(" + std::to_string(
        static_cast<typename std::underlying_type<T>::type>(t)
      ) + ")";
    } else if constexpr(std::is_same_v<std::remove_cv_t<T>, bool>) {
      return t ? "true" : "false";
    } else if constexpr(is_printable_v<T>) {
      return t;
    } else if constexpr(is_exception_v<T>) {
      std::ostringstream ss;
      ss << type_name(t) << "(" << to_printable(t.what()) << ")";
      return ss.str();
    } else if constexpr(is_iterable_v<T>) {
      return to_printable(std::begin(t), std::end(t));
    } else {
      return type_name<T>();
    }
  }

  template<typename T>
  inline auto to_printable(const volatile T &t) {
    return to_printable(const_cast<const T &>(t));
  }

  // These need to be last in order for the `to_printable()` calls inside to
  // pick up all the above implementations (plus any others via ADL).

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
