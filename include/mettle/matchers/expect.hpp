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
  public:
    expectation_error(const std::string &what, detail::source_location loc) :
      runtime_error(what), location_(std::move(loc)) {}
    expectation_error(std::string desc, const std::string &what,
                      detail::source_location loc) :
      runtime_error(what), desc_(std::move(desc)), location_(std::move(loc)) {}

    const std::string & desc() const {
      return desc_;
    }

    const detail::source_location & location() const {
      return location_;
    }
  private:
    std::string desc_;
    detail::source_location location_;
  };

  template<typename T, any_matcher Matcher>
  void expect(T &&value, const Matcher &matcher,
              detail::source_location loc =
                detail::source_location::current()) {
    if(auto m = matcher(value); m == false) {
      std::ostringstream ss;
      ss << "expected: " << matcher.desc() << std::endl
         << "actual:   " << matcher_message(m, value);
      throw expectation_error(ss.str(), loc);
    }
  }

  template<typename T, any_matcher Matcher>
  void expect(const std::string &desc, T &&value, const Matcher &matcher,
              detail::source_location loc =
                detail::source_location::current()) {
    if(auto m = matcher(value); m == false) {
      std::ostringstream ss;
      ss << "expected: " << matcher.desc() << std::endl
         << "actual:   " << matcher_message(m, value);
      throw expectation_error(desc, ss.str(), loc);
    }
  }

} // namespace mettle

#endif
