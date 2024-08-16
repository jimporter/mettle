#ifndef INC_METTLE_MATCHERS_EXPECT_HPP
#define INC_METTLE_MATCHERS_EXPECT_HPP

#include <sstream>
#include <string>
#include <type_traits>

#include "core.hpp"
#include "result.hpp"
#include "../detail/source_location.hpp"

namespace mettle {

  class expectation_error : public std::runtime_error {
    using std::runtime_error::runtime_error;
  };

  namespace detail {
    inline void
    expect_fail(std::ostream &os, const detail::source_location &loc,
                std::string_view user_desc, std::string_view matcher_desc) {
      if(!user_desc.empty())
        os << user_desc << " (";
      os << loc.file_name() << ":" << loc.line();
      if(!user_desc.empty())
        os << ")";
      os << std::endl << "expected: " << matcher_desc
         << std::endl << "actual:   ";
    }
  }

  template<typename T, any_matcher Matcher>
  void expect(T &&value, const Matcher &matcher,
              detail::source_location loc =
                detail::source_location::current()) {
    if(auto m = matcher(value); m == false) {
      std::ostringstream ss;
      detail::expect_fail(ss, loc, "", matcher.desc());
      ss << matcher_message(m, value);
      throw expectation_error(ss.str());
    }
  }

  template<typename T, any_matcher Matcher>
  void expect(const std::string &desc, T &&value, const Matcher &matcher,
              detail::source_location loc =
                detail::source_location::current()) {
    if(auto m = matcher(value); m == false) {
      std::ostringstream ss;
      detail::expect_fail(ss, loc, desc, matcher.desc());
      ss << matcher_message(m, value);
      throw expectation_error(ss.str());
    }
  }

} // namespace mettle

#endif
