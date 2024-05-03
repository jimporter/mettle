#ifndef INC_METTLE_OUTPUT_STRING_HPP
#define INC_METTLE_OUTPUT_STRING_HPP

#include <codecvt>
#include <ostream>
#include <string>
#include <string_view>

namespace mettle {

  namespace detail {

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
      } else if(c == delim || c == escape) {
        os << escape << c;
      } else {
        os << c;
      }
    }

  }

  template<typename Char, typename Traits,
           typename Alloc = std::allocator<Char>>
  std::basic_string<Char, Traits, Alloc>
  escape_string(const std::basic_string_view<Char, Traits> &s,
                Char delim = '"') {
    std::basic_ostringstream<Char, Traits, Alloc> ss;
    ss << std::hex << delim;
    for(const auto &c : s)
      detail::escape_char(ss, c, delim);
    ss << delim;
    return ss.str();
  }

  template<typename Char, typename Traits, typename Alloc>
  std::basic_string<Char, Traits>
  escape_string(const std::basic_string<Char, Traits, Alloc> &s,
                Char delim = '"') {
    return escape_string<Char, Traits, Alloc>(
      std::basic_string_view<Char, Traits>(s), delim
    );
  }

  template<typename Char, typename Traits = std::char_traits<Char>,
           typename Alloc = std::allocator<Char>>
  std::basic_string<Char, Traits, Alloc>
  escape_string(const Char *s, Char delim = '"') {
    return escape_string<Char, Traits, Alloc>(
      std::basic_string_view<Char>(s), delim
    );
  }

  inline std::string_view
  string_convert(const std::string_view &s) {
    return s;
  }

  inline std::string_view
  string_convert(const std::basic_string_view<unsigned char> &s) {
    auto begin = reinterpret_cast<const char *>(s.data());
    return std::string_view(begin, s.size());
  }

  inline std::string_view
  string_convert(const std::basic_string_view<signed char> &s) {
    auto begin = reinterpret_cast<const char *>(s.data());
    return std::string_view(begin, s.size());
  }

// Ignore warnings about deprecated <codecvt>.
#if defined(_MSC_VER) && !defined(__clang__)
#  pragma warning(push)
#  pragma warning(disable:4996)
#elif defined(__GNUG__)
#  pragma GCC diagnostic push
#  pragma GCC diagnostic ignored "-Wdeprecated"
#endif

  inline std::string
  string_convert(const std::wstring_view &s) {
    std::wstring_convert<std::codecvt_utf8<wchar_t>, wchar_t> conv;
    return conv.to_bytes(s.data(), s.data() + s.size());
  }

  inline std::string
  string_convert(const std::u16string_view &s) {
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
  string_convert(const std::u32string_view &s) {
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

#if defined(_MSC_VER) && !defined(__clang__)
#  pragma warning(pop)
#elif defined(__GNUG__)
#  pragma GCC diagnostic pop
#endif

  }

} // namespace mettle

#endif
