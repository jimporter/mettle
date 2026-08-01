#ifndef INC_METTLE_OUTPUT_STRING_HPP
#define INC_METTLE_OUTPUT_STRING_HPP

#include <cstddef>
#include <ostream>
#include <string>
#include <string_view>

#if __has_include(<boost/locale/encoding_utf.hpp>)
#  include <boost/locale/encoding_utf.hpp>

namespace mettle {

  inline std::string
  convert_string(const std::wstring_view &s) {
    return boost::locale::conv::utf_to_utf<char>(s.data(), s.data() + s.size());
  }

  inline std::string
  convert_string(const std::u16string_view &s) {
    return boost::locale::conv::utf_to_utf<char>(s.data(), s.data() + s.size());
  }

  inline std::string
  convert_string(const std::u32string_view &s) {
    return boost::locale::conv::utf_to_utf<char>(s.data(), s.data() + s.size());  }

} // namespace mettle

#endif

namespace mettle {

  inline std::string_view
  convert_string(const std::string_view &s) {
    return s;
  }

  inline std::string_view
  convert_string(const std::u8string_view &s) {
    return std::string_view(reinterpret_cast<const char *>(s.data()), s.size());
  }

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

  template<typename T>
  concept string_convertible = requires(T &t) { convert_string(t); };

  template<string_convertible String>
  inline auto
  represent_string(const String &s, char delim = '"') {
    return escape_string(convert_string(s), delim);
  }

} // namespace mettle

#endif
