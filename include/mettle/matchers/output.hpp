#ifndef INC_METTLE_MATCHERS_OUTPUT_HPP
#define INC_METTLE_MATCHERS_OUTPUT_HPP

#include <codecvt>
#include <locale>
#include <iomanip>
#include <map>
#include <sstream>
#include <type_traits>
#include <typeinfo>

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
  template<typename Tuple, typename Func, typename Val,
           size_t N = std::tuple_size<Tuple>::value>
  struct do_reduce {
    auto operator ()(const Tuple &tuple, const Func &reducer,
                     Val &&value) {
      constexpr auto i = std::tuple_size<Tuple>::value - N;
      bool early_exit = false;
      auto result = reducer(
        std::forward<Val>(value), std::get<i>(tuple), early_exit
      );
      if(early_exit)
        return result;
      return do_reduce<Tuple, Func, Val, N-1>()(
        tuple, reducer, std::forward<Val>(result)
      );
    }
  };

  template<typename Tuple, typename Func, typename Val>
  struct do_reduce<Tuple, Func, Val, 1> {
    auto operator ()(const Tuple &tuple, const Func &reducer,
                     Val &&value) {
      constexpr auto i = std::tuple_size<Tuple>::value - 1;
      bool early_exit = false;
      return reducer(std::forward<Val>(value), std::get<i>(tuple), early_exit);
    }
  };

  template<typename Tuple, typename Func, typename Val>
  struct do_reduce<Tuple, Func, Val, 0> {
    auto operator ()(const Tuple &, const Func &, Val &&value) {
      return value;
    }
  };

  template<typename Tuple, typename Func, typename Val>
  auto reduce_tuple(const Tuple &tuple, const Func &reducer, Val &&initial) {
    return do_reduce<Tuple, Func, Val>()(
      tuple, reducer, std::forward<Val>(initial)
    );
  }

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

template<typename T>
class is_printable {
  template<typename U> struct always_bool { typedef bool type; };

  template<typename U>
  static constexpr typename always_bool<
    decltype( std::declval<std::ostream&>() << std::declval<U>() )
  >::type check_(int) {
    return true;
  }
  template<typename U>
  static constexpr bool check_(...) {
    return false;
  }
public:
  static const bool value = check_<T>(0);
};

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
class is_iterable {
  template<typename U> struct always_bool { typedef bool type; };

  template<typename U>
  static constexpr typename always_bool<
    decltype( std::begin(std::declval<U>()), std::end(std::declval<U>()) )
  >::type check_(int) {
    return true;
  }
  template<typename U>
  static constexpr bool check_(...) {
    return false;
  }
public:
  static const bool value = check_<T>(0);
};

template<typename T, size_t N>
class is_iterable<T[N]> : public std::true_type {};

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
//     if is_iterable -> iterable
//     else if is_enum -> enum (class)
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

inline std::string to_printable(const std::exception &e) {
  std::ostringstream ss;
  ss << "exception(" << detail::escape_str(e.what()) << ")";
  return ss.str();
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
  try {
    return typeid(t).name();
  }
  catch(...) {
    return "...";
  }
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
  !is_printable<T>::value && !is_iterable<T>::value && !std::is_enum<T>::value,
  std::string
>::type {
  try {
    return typeid(t).name();
  }
  catch(...) {
    return "...";
  }
}

// Enum classes

template<typename T>
constexpr inline auto to_printable(T t) -> typename std::enable_if<
  !is_printable<T>::value && std::is_enum<T>::value, std::string
>::type {
  return to_printable_boolish(t);
}

// Iterables

template<typename T>
auto to_printable(const T &v) -> typename std::enable_if<
  !is_printable<T>::value && is_iterable<T>::value, std::string
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
    ss << "[";
    reduce_tuple(tuple, [&ss](bool first, const auto &x, bool &) {
      if(!first)
        ss << ", ";
      ss << to_printable(x);
      return false;
    }, true);
    ss << "]";
    return ss.str();
  }
}

} // namespace mettle

#endif
