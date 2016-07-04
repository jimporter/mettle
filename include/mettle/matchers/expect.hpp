#ifndef INC_METTLE_MATCHERS_EXPECT_HPP
#define INC_METTLE_MATCHERS_EXPECT_HPP

#include <sstream>
#include <string>

#include "core.hpp"
#include "result.hpp"
#include "detail/source_location.hpp"

#ifndef METTLE_NO_MACROS
#  ifdef METTLE_NO_SOURCE_LOCATION
#    define METTLE_EXPECT(...) mettle::expect(                       \
       __VA_ARGS__,                                                  \
       METTLE_SOURCE_LOCATION::current(__FILE__, __func__, __LINE__) \
     )
#  else
#    define METTLE_EXPECT(...) mettle::expect(__VA_ARGS__)
#  endif
#endif

namespace mettle {

class expectation_error : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

template<typename T, typename Matcher>
void expect(const T &value, const Matcher &matcher,
            METTLE_SOURCE_LOCATION loc = METTLE_SOURCE_LOCATION::current()) {
  static_assert(is_matcher_v<Matcher>, "expected a matcher for argument 2");

  auto m = matcher(value);
  if(m == false) {
    std::ostringstream ss;
    if(loc.line())
      ss << loc.file_name() << ":" << loc.line() << std::endl;
    ss << "expected: " << matcher.desc() << std::endl
       << "actual:   " << matcher_message(m, value);
    throw expectation_error(ss.str());
  }
}

template<typename T, typename Matcher>
void expect(const std::string &desc, const T &value, const Matcher &matcher,
            METTLE_SOURCE_LOCATION loc = METTLE_SOURCE_LOCATION::current()) {
  static_assert(is_matcher_v<Matcher>, "expected a matcher for argument 3");

  auto m = matcher(value);
  if(m == false) {
    std::ostringstream ss;
    ss << desc;
    if(loc.line())
      ss << " (" << loc.file_name() << ":" << loc.line() << ")";
    ss << std::endl << "expected: " << matcher.desc() << std::endl
       << "actual:   " << matcher_message(m, value);
    throw expectation_error(ss.str());
  }
}

} // namespace mettle

#endif
