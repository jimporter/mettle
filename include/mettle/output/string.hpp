#ifndef INC_METTLE_OUTPUT_STRING_HPP
#define INC_METTLE_OUTPUT_STRING_HPP

#include <cstddef>
#include <codecvt>
#include <ostream>
#include <string>
#include <string_view>

namespace mettle {

  namespace detail {

    inline void escape_char(std::ostream &os, char c, char delim) {
      const char escape = '\\';
      if(c < 32 || c == 0x7f) {
        os << escape;
        switch(c) {
        case '\0': os << '0'; break;
        case '\a': os << 'a'; break;
        case '\b': os << 'b'; break;
        case '\f': os << 'f'; break;
        case '\n': os << 'n'; break;
        case '\r': os << 'r'; break;
        case '\t': os << 't'; break;
        case '\v': os << 'v'; break;
        default:   os << 'x' << static_cast<unsigned long>(c);
        }
      } else if(c == delim || c == escape) {
        os << escape << c;
      } else {
        os << c;
      }
    }

  }

  inline std::string
  escape_string(const std::string_view &s, char delim = '"') {
    std::ostringstream ss;
    ss << std::hex << delim;
    for(const auto &c : s)
      detail::escape_char(ss, c, delim);
    ss << delim;
    return ss.str();
  }

  inline std::string_view
  convert_string(const std::string_view &s) {
    return s;
  }

#if __cpp_char8_t
  inline std::string_view
  convert_string(const std::u8string_view &s) {
    return std::string_view(reinterpret_cast<const char *>(s.data()), s.size());
  }
#endif

// Ignore warnings about deprecated <codecvt>.
#if defined(_MSC_VER) && !defined(__clang__)
#  pragma warning(push)
#  pragma warning(disable:4996)
#elif defined(__GNUG__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated"
#endif

  inline std::string
  convert_string(const std::wstring_view &s) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
    return conv.to_bytes(s.data(), s.data() + s.size());
  }

  inline std::string
  convert_string(const std::u16string_view &s) {
#if defined(_MSC_VER) && !defined(__clang__)
    // MSVC's codecvt expects uint16_t instead of char16_t because char16_t
    // used to just be a typedef of uint16_t.
    std::wstring_convert<std::codecvt_utf8_utf16<std::uint16_t>,
                         std::uint16_t> conv;
    return conv.to_bytes(
      reinterpret_cast<const std::uint16_t *>(s.data()),
      reinterpret_cast<const std::uint16_t *>(s.data() + s.size())
    );
#else
    std::wstring_convert<std::codecvt_utf8_utf16<char16_t>, char16_t> conv;
    return conv.to_bytes(s.data(), s.data() + s.size());
#endif
  }

  inline std::string
  convert_string(const std::u32string_view &s) {
#if defined(_MSC_VER) && !defined(__clang__)
    // MSVC's codecvt expects uint32_t instead of char32_t because char32_t
    // used to just be a typedef of uint32_t.
    std::wstring_convert<std::codecvt_utf8<std::uint32_t>, std::uint32_t> conv;
    return conv.to_bytes(
      reinterpret_cast<const std::uint32_t *>(s.data()),
      reinterpret_cast<const std::uint32_t *>(s.data() + s.size())
    );
#else
    std::wstring_convert<std::codecvt_utf8<char32_t>, char32_t> conv;
    return conv.to_bytes(s.data(), s.data() + s.size());
#endif
  }

#if defined(_MSC_VER) && !defined(__clang__)
#  pragma warning(pop)
#elif defined(__GNUG__)
#  pragma GCC diagnostic pop
#endif

  template<typename, typename = std::void_t<>>
  struct is_string_convertible : std::false_type {};

  template<typename T>
  struct is_string_convertible<T, std::void_t<
    decltype(convert_string(std::declval<T&>()))
  >> : std::true_type {};

  template<typename T>
  inline constexpr bool is_string_convertible_v =
    is_string_convertible<T>::value;

  template<typename String>
  inline auto
  represent_string(const String &s, char delim = '"') {
    static_assert(is_string_convertible_v<String>);
    return escape_string(convert_string(s), delim);
  }

} // namespace mettle

#endif
