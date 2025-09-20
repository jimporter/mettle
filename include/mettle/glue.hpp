#ifndef INC_METTLE_GLUE_HPP
#define INC_METTLE_GLUE_HPP

#include <algorithm>

#include "suite/make_suite.hpp"
#include "suite/global_suite.hpp"
#include "matchers/expect.hpp"

namespace mettle {

  template<typename ...Fixture, typename ...Args>
  inline auto
  make_suite(std::string_view name, const attributes &attrs, Args &&...args) {
    return make_basic_suite<expectation_error, Fixture...>(
      name, attrs, std::forward<Args>(args)...
    );
  }

  template<typename ...Fixture, typename ...Args>
  inline auto
  make_suite(std::string_view name, Args &&...args) {
    return make_basic_suite<expectation_error, Fixture...>(
      name, std::forward<Args>(args)...
    );
  }

  template<typename ...Fixture, typename ...Args>
  inline auto
  make_suites(std::string_view name, const attributes &attrs, Args &&...args) {
    return make_basic_suites<expectation_error, Fixture...>(
      name, attrs, std::forward<Args>(args)...
    );
  }

  template<typename ...Fixture, typename ...Args>
  inline auto
  make_suites(std::string_view name, Args &&...args) {
    return make_basic_suites<expectation_error, Fixture...>(
      name, std::forward<Args>(args)...
    );
  }

  template<typename ...T>
  using suite = basic_suite<expectation_error, T...>;

} // namespace mettle

#endif
