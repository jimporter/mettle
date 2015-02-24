#ifndef INC_METTLE_OUTPUT_TO_PRINTABLE_HPP
#define INC_METTLE_OUTPUT_TO_PRINTABLE_HPP

#include <cstdint>
#include <locale>
#include <iomanip>
#include <sstream>
#include <string>

#include "string.hpp"
#include "traits.hpp"
#include "type_name.hpp"
#include "../detail/string_algorithm.hpp"
#include "../detail/tuple_algorithm.hpp"

// XXX: Remove this when MSVC supports "optional" constexpr on templates.
#if !defined(_MSC_VER) || defined(__clang__)
#  define METTLE_CONSTEXPR constexpr
#else
#  define METTLE_CONSTEXPR
#endif

namespace mettle {

// The to_printable overloads below are rather complicated, to say the least; we
// need to be extra-careful to ensure that things which are convertible to bool
// don't erroneously get printed out *as* bools. As such, if a type is
// "bool-ish" (convertible to bool, but not a non-bool arithmetic type), we
// delegate to to_printable_boolish. Here's the basic structure:
//
// to_printable:
//   if is_printable
//     if is_boolish -> to_printable_boolish
//     else -> pass-through
//   else
//     if is_enum -> enum (class)
//     else if is_exception -> exception
//     else if is_iterable -> iterable
//     else -> fallback
//
// to_printable_boolish:
//   if is_bool -> bool
//   if is_enum -> enum
//   if is_pointer
//     if is_any_char -> c string
//     else if is_function -> function pointer
//     else -> pointer
//   if !is_scalar -> fallback
//
// We also have an overload for array types, which is selected if it's *not* an
// array of char (arrays of char ultimately get selected by a const char *
// overload). Finally, we have a few non-generic overloads for nullptrs,
// std::strings, and std::pairs/std::tuples.

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
to_printable(const METTLE_STRING_VIEW<Char, Traits> &s) {
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

// Helper for bool-ish types

template<typename T>
inline auto to_printable_boolish(T b) -> typename std::enable_if<
  std::is_same<typename std::remove_cv<T>::type, bool>::value, std::string
>::type {
  return b ? "true" : "false";
}

template<typename T>
auto to_printable_boolish(T t) -> typename std::enable_if<
  std::is_enum<T>::value, std::string
>::type {
  return type_name<T>() + "(" + std::to_string(
    static_cast<typename std::underlying_type<T>::type>(t)
  ) + ")";
}

template<typename T>
METTLE_CONSTEXPR inline auto to_printable_boolish(const T *t) ->
typename std::enable_if<!std::is_function<T>::value, const T *>::type {
  return t;
}

template<typename Ret, typename ...Args>
inline auto to_printable_boolish(Ret (*)(Args...)) {
  return type_name<Ret(Args...)>();
}

// XXX: These don't work for volatile strings.

inline std::string to_printable_boolish(const char *s) {
  if(!s) return detail::null_str();
  return escape_string(s);
}

inline std::string to_printable_boolish(const unsigned char *s) {
  if(!s) return detail::null_str();
  return escape_string(reinterpret_cast<const char*>(s));
}

inline std::string to_printable_boolish(const signed char *s) {
  if(!s) return detail::null_str();
  return escape_string(reinterpret_cast<const char*>(s));
}

inline std::string to_printable_boolish(const wchar_t *s) {
  if(!s) return detail::null_str();
  return escape_string(string_convert(s));
}

inline std::string to_printable_boolish(const char16_t *s) {
  if(!s) return detail::null_str();
  return escape_string(string_convert(s));
}

inline std::string to_printable_boolish(const char32_t *s) {
  if(!s) return detail::null_str();
  return escape_string(string_convert(s));
}

template<typename T>
auto to_printable_boolish(const T &t) -> typename std::enable_if<
  !std::is_scalar<typename std::remove_reference<T>::type>::value, std::string
>::type {
  return type_name(t);
}

// Pass-through

template<typename T>
METTLE_CONSTEXPR auto to_printable(const T &t) -> typename std::enable_if<
  is_printable<T>::value && !is_boolish<T>::value, const T &
>::type {
  return t;
}

// Bool-ish types

template<typename T>
METTLE_CONSTEXPR inline auto
to_printable(const T &t) -> typename std::enable_if<
  is_printable<T>::value && is_boolish<T>::value,
  decltype(to_printable_boolish(t))
>::type {
  return to_printable_boolish(t);
}

// Fallback

template<typename T>
auto to_printable(const T &t) -> typename std::enable_if<
  !is_printable<T>::value && !std::is_enum<T>::value &&
  !is_exception<T>::value && !is_iterable<T>::value,
  std::string
>::type {
  return type_name(t);
}

// Enum classes

template<typename T>
METTLE_CONSTEXPR inline auto to_printable(T t) -> typename std::enable_if<
  !is_printable<T>::value && std::is_enum<T>::value, std::string
>::type {
  return to_printable_boolish(t);
}

// Exceptions

template<typename T>
inline auto to_printable(const T &e) -> typename std::enable_if<
  !is_printable<T>::value && is_exception<T>::value, std::string
>::type {
  std::ostringstream ss;
  ss << type_name(e) << "(" << to_printable(e.what()) << ")";
  return ss.str();
}

// Iterables

template<typename T>
std::string to_printable(T begin, T end);

template<typename T>
auto to_printable(const T &v) -> typename std::enable_if<
  !is_printable<T>::value && !is_exception<T>::value && is_iterable<T>::value,
  std::string
>::type {
  return to_printable(std::begin(v), std::end(v));
}

template<typename T, std::size_t N>
auto to_printable(const T (&v)[N]) -> typename std::enable_if<
  !is_any_char<T>::value, std::string
>::type {
  return to_printable(std::begin(v), std::end(v));
}

// Pairs/Tuples

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
