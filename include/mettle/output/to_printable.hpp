#ifndef INC_METTLE_OUTPUT_TO_PRINTABLE_HPP
#define INC_METTLE_OUTPUT_TO_PRINTABLE_HPP

#include <codecvt>
#include <locale>
#include <iomanip>
#include <sstream>

#include "traits.hpp"
#include "type_name.hpp"
#include "../detail/tuple_algorithm.hpp"

// Try to use N4082's string_view class, or fall back to Boost's.
#ifdef __has_include
#  if __has_include(<experimental/string_view>)
#    include <experimental/string_view>
#    define METTLE_STRING_VIEW std::experimental::basic_string_view
#  else
#    include <boost/utility/string_ref.hpp>
#    define METTLE_STRING_VIEW boost::basic_string_ref
#  endif
#else
#  include <boost/utility/string_ref.hpp>
#  define METTLE_STRING_VIEW boost::basic_string_ref
#endif

namespace mettle {

namespace detail {
  template<typename T>
  std::string stringify_iterable(T begin, T end);

  template<typename T>
  std::string stringify_tuple(const T &tuple);

  template<typename Char, typename Traits>
  void escape_char(std::basic_ostream<Char, Traits> &os, Char c, Char delim) {
    const char escape = '\\';
    if(c < 32 || c == 0x7f) {
      os << escape;
      switch(c) {
      case '\0': os << os.widen('0'); break;
      case '\a': os << os.widen('a'); break;
      case '\b': os << os.widen('b'); break;
      case '\f': os << os.widen('f'); break;
      case '\n': os << os.widen('n'); break;
      case '\r': os << os.widen('r'); break;
      case '\t': os << os.widen('t'); break;
      case '\v': os << os.widen('v'); break;
      default:   os << os.widen('x') << static_cast<unsigned long>(c);
      }
    }
    else if(c == delim || c == escape) {
      os << escape << c;
    }
    else {
      os << c;
    }
  }

  template<typename Char, typename Traits>
  std::basic_string<Char, Traits>
  escape_str(const METTLE_STRING_VIEW<Char, Traits> &s, Char delim = '"') {
    std::basic_ostringstream<Char, Traits> ss;
    ss << std::hex << delim;
    for(const auto &c : s)
      escape_char(ss, c, delim);
    ss << delim;
    return ss.str();
  }

  template<typename Char, typename Traits>
  std::basic_string<Char, Traits>
  escape_str(const std::basic_string<Char, Traits> &s, Char delim = '"') {
    return escape_str(METTLE_STRING_VIEW<Char, Traits>(s), delim);
  }

  template<typename Char>
  std::basic_string<Char>
  escape_str(const Char *s, Char delim = '"') {
    return escape_str(METTLE_STRING_VIEW<Char>(s), delim);
  }
}

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

inline std::string to_printable(const std::string &s) {
  return detail::escape_str(s);
}

inline std::string to_printable(const std::wstring &s) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
  return detail::escape_str(conv.to_bytes(s));
}

inline std::string to_printable(const std::u16string &s) {
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
  return detail::escape_str(conv.to_bytes(s));
}

inline std::string to_printable(const std::u32string &s) {
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
  return detail::escape_str(conv.to_bytes(s));
}

inline std::string to_printable(char c) {
  return detail::escape_str(std::string(1, c), '\'');
}

inline std::string to_printable(unsigned char c) {
  return detail::escape_str(std::string(1, c), '\'');
}

inline std::string to_printable(signed char c) {
  return detail::escape_str(std::string(1, c), '\'');
}

inline std::string to_printable(wchar_t c) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
  return detail::escape_str(conv.to_bytes(std::wstring(1, c)), '\'');
}

inline std::string to_printable(char16_t c) {
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
  return detail::escape_str(conv.to_bytes(std::u16string(1, c)), '\'');
}

inline std::string to_printable(char32_t c) {
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
  return detail::escape_str(conv.to_bytes(std::u32string(1, c)), '\'');
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
constexpr inline auto to_printable_boolish(const T *t) ->
typename std::enable_if<!std::is_function<T>::value, const T *>::type {
  return t;
}

template<typename Ret, typename ...Args>
inline auto to_printable_boolish(Ret (*)(Args...)) {
  return type_name<Ret(Args...)>();
}

// XXX: These don't work for volatile strings.

inline std::string to_printable_boolish(const char *s) {
  return detail::escape_str(s);
}

inline std::string to_printable_boolish(const unsigned char *s) {
  return detail::escape_str(reinterpret_cast<const char*>(s));
}

inline std::string to_printable_boolish(const signed char *s) {
  return detail::escape_str(reinterpret_cast<const char*>(s));
}

inline std::string to_printable_boolish(const wchar_t *s) {
  std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
  return detail::escape_str(conv.to_bytes(s));
}

inline std::string to_printable_boolish(const char16_t *s) {
  std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
  return detail::escape_str(conv.to_bytes(s));
}

inline std::string to_printable_boolish(const char32_t *s) {
  std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
  return detail::escape_str(conv.to_bytes(s));
}

template<typename T>
auto to_printable_boolish(const T &t) -> typename std::enable_if<
  !std::is_scalar<typename std::remove_reference<T>::type>::value, std::string
>::type {
  return type_name(t);
}

// Pass-through

template<typename T>
constexpr auto to_printable(const T &t) -> typename std::enable_if<
  is_printable<T>::value && !is_boolish<T>::value, const T &
>::type {
  return t;
}

// Bool-ish types

template<typename T>
constexpr inline auto to_printable(const T &t) -> typename std::enable_if<
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
constexpr inline auto to_printable(T t) -> typename std::enable_if<
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
auto to_printable(const T &v) -> typename std::enable_if<
  !is_printable<T>::value && !is_exception<T>::value && is_iterable<T>::value,
  std::string
>::type {
  return detail::stringify_iterable(std::begin(v), std::end(v));
}

template<typename T, size_t N>
auto to_printable(const T (&v)[N]) -> typename std::enable_if<
  !is_any_char<T>::value, std::string
>::type {
  return detail::stringify_iterable(std::begin(v), std::end(v));
}

// Pairs/Tuples

template<typename T, typename U>
std::string to_printable(const std::pair<T, U> &pair) {
  return detail::stringify_tuple(pair);
}

template<typename ...T>
std::string to_printable(const std::tuple<T...> &tuple) {
  return detail::stringify_tuple(tuple);
}

namespace detail {
  template<typename T>
  std::string stringify_iterable(T begin, T end) {
    std::ostringstream ss;
    ss << "[";
    if(begin != end) {
      ss << to_printable(*begin);
      for(++begin; begin != end; ++begin)
        ss << ", " << to_printable(*begin);
    }
    ss << "]";
    return ss.str();
  }

  template<typename T>
  std::string stringify_tuple(const T &tuple) {
    std::ostringstream ss;
    bool first = true;
    ss << "[";
    tuple_for_each(tuple, [&ss, &first](const auto &x) {
      if(!first)
        ss << ", ";
      ss << to_printable(x);
      first = false;
    });
    ss << "]";
    return ss.str();
  }
}

} // namespace mettle

#endif
