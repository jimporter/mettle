#ifndef INC_METTLE_MATCHERS_REGEX_HPP
#define INC_METTLE_MATCHERS_REGEX_HPP

#include <regex>

#include "core.hpp"

namespace mettle {

  template<typename Char, typename Traits, typename Alloc>
  auto regex_match(const std::basic_string<Char, Traits, Alloc> &ex,
                   std::regex_constants::syntax_option_type syntax =
                     std::regex_constants::ECMAScript,
                   std::regex_constants::match_flag_type match = {}) {
    std::ostringstream ss;
    ss << "matched " << escape_string(string_convert(ex), '/');
    if(syntax & std::regex_constants::icase)
      ss << "i";

    return basic_matcher(
      [ex = std::basic_regex<Char>(ex, syntax), match](const auto &actual) {
        return std::regex_match(actual, ex, match);
      }, ss.str()
    );
  }

  template<typename Char>
  auto regex_match(const Char *ex,
                   std::regex_constants::syntax_option_type syntax =
                     std::regex_constants::ECMAScript,
                   std::regex_constants::match_flag_type match = {}) {
    return regex_match(std::basic_string<Char>(ex), syntax, match);
  }

  template<typename Char, typename Traits, typename Alloc>
  auto regex_search(const std::basic_string<Char, Traits, Alloc> &ex,
                    std::regex_constants::syntax_option_type syntax =
                      std::regex_constants::ECMAScript,
                    std::regex_constants::match_flag_type match = {}) {
    std::ostringstream ss;
    ss << "searched " << escape_string(string_convert(ex), '/');
    if(syntax & std::regex_constants::icase)
      ss << "i";

    return basic_matcher(
      [ex = std::basic_regex<Char>(ex, syntax), match](const auto &actual) {
        return std::regex_search(actual, ex, match);
      }, ss.str()
    );
  }

  template<typename Char>
  auto regex_search(const Char *ex,
                    std::regex_constants::syntax_option_type syntax =
                      std::regex_constants::ECMAScript,
                    std::regex_constants::match_flag_type match = {}) {
    return regex_search(std::basic_string<Char>(ex), syntax, match);
  }

} // namespace mettle

#endif
