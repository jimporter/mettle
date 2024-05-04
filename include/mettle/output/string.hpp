#ifndef INC_METTLE_OUTPUT_STRING_HPP
#define INC_METTLE_OUTPUT_STRING_HPP

#include <cstddef>
#include <codecvt>
#include <ostream>
#include <string>
#include <string_view>

namespace mettle {

  inline std::string
  escape_string(const std::string_view &s, char delim = '"') {
    std::ostringstream ss;
    ss << std::hex << delim;
    for(auto c : s) {
      const char escape = '\\';
      if(c < 32 || c == 0x7f) {
        ss << escape;
        switch(c) {
        case '\0': ss << '0'; break;
        case '\a': ss << 'a'; break;
        case '\b': ss << 'b'; break;
        case '\f': ss << 'f'; break;
        case '\n': ss << 'n'; break;
        case '\r': ss << 'r'; break;
        case '\t': ss << 't'; break;
        case '\v': ss << 'v'; break;
        default:   ss << 'x' << static_cast<unsigned long>(c);
        }
      } else if(c == delim || c == escape) {
        ss << escape << c;
      } else {
        ss << c;
      }
    }
    ss << delim;
    return ss.str();
  }

  inline std::string_view
  convert_string(const std::string_view &s) {
    return s;
  }

  inline std::string_view
  convert_string(const std::basic_string_view<unsigned char> &s) {
    auto begin = reinterpret_cast<const char *>(s.data());
    return std::string_view(begin, s.size());
  }

  inline std::string_view
  convert_string(const std::basic_string_view<signed char> &s) {
    auto begin = reinterpret_cast<const char *>(s.data());
    return std::string_view(begin, s.size());
  }

  inline std::string_view
  convert_string(const std::basic_string_view<std::byte> &s) {
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

  namespace detail {
    template<typename, typename = std::void_t<>>
    struct is_string_convertible : std::false_type {};

    template<typename T>
    struct is_string_convertible<T, std::void_t<
      decltype(convert_string(std::declval<T&>()))
    >> : std::true_type {};
  }

  template<typename String>
  inline auto
  represent_string(const String &s,
                   [[maybe_unused]] char delim = '"') { // Silence GCC < 10.
    if constexpr(detail::is_string_convertible<String>::value) {
      return escape_string(convert_string(s), delim);
    } else {
      return std::string("(unrepresentable string)");
    }
  }

} // namespace mettle

#endif
