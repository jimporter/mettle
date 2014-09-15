#ifndef INC_METTLE_MATCHERS_EXPECT_HPP
#define INC_METTLE_MATCHERS_EXPECT_HPP

#include <sstream>
#include <string>

namespace mettle {

class expectation_error : public std::runtime_error {
  using std::runtime_error::runtime_error;
};

template<typename T, typename Matcher>
void expect(const T &value, const Matcher &matcher) {
  if(!matcher(value)) {
    std::ostringstream ss;
    ss << "expected: " << matcher.desc() << std::endl
       << "actual:   " << to_printable(value);
    throw expectation_error(ss.str());
  }
}

template<typename T, typename Matcher>
void expect(const std::string &desc, const T &value, const Matcher &matcher) {
  if(!matcher(value)) {
    std::ostringstream ss;
    ss << desc << std::endl
       << "expected: " << matcher.desc() << std::endl
       << "actual:   " << to_printable(value);
    throw expectation_error(ss.str());
  }
}

} // namespace mettle

#endif
