#ifndef INC_METTLE_GLUE_HPP
#define INC_METTLE_GLUE_HPP

#include <algorithm>

#include "suite/make_suite.hpp"
#include "suite/global_suite.hpp"
#include "matchers/expect.hpp"

namespace mettle {

  namespace detail {
    constexpr inline std::size_t at_least_one(std::size_t a) {
      return std::max(a, std::size_t(1));
    }
  }

  template<typename ...Fixture, typename Factory, typename F>
  inline runnable_suite
  make_suite(const std::string &name, const attributes &attrs,
             Factory &&factory, F &&f) {
    return make_basic_suite<expectation_error, Fixture...>(
      name, attrs, std::forward<Factory>(factory), std::forward<F>(f)
    );
  }

  template<typename ...Fixture, typename Factory, typename F>
  inline runnable_suite
  make_suite(const std::string &name, Factory &&factory, F &&f) {
    return make_basic_suite<expectation_error, Fixture...>(
      name, std::forward<Factory>(factory), std::forward<F>(f)
    );
  }

  template<typename ...Fixture, typename F>
  inline runnable_suite
  make_suite(const std::string &name, const attributes &attrs, F &&f) {
    return make_suite<Fixture...>(name, attrs, auto_factory,
                                  std::forward<F>(f));
  }

  template<typename ...Fixture, typename F>
  inline runnable_suite
  make_suite(const std::string &name, F &&f) {
    return make_suite<Fixture...>(name, auto_factory, std::forward<F>(f));
  }

  template<typename ...Fixture, typename Factory, typename F>
  inline std::array<runnable_suite, detail::at_least_one(sizeof...(Fixture))>
  make_suites(const std::string &name, const attributes &attrs,
             Factory &&factory, F &&f) {
    return make_basic_suites<expectation_error, Fixture...>(
      name, attrs, std::forward<Factory>(factory), std::forward<F>(f)
    );
  }

  template<typename ...Fixture, typename Factory, typename F>
  inline std::array<runnable_suite, detail::at_least_one(sizeof...(Fixture))>
  make_suites(const std::string &name, Factory &&factory, F &&f) {
    return make_basic_suites<expectation_error, Fixture...>(
      name, std::forward<Factory>(factory), std::forward<F>(f)
    );
  }

  template<typename ...Fixture, typename F>
  inline std::array<runnable_suite, detail::at_least_one(sizeof...(Fixture))>
  make_suites(const std::string &name, const attributes &attrs, F &&f) {
    return make_suites<Fixture...>(name, attrs, auto_factory,
                                   std::forward<F>(f));
  }

  template<typename ...Fixture, typename F>
  inline std::array<runnable_suite, detail::at_least_one(sizeof...(Fixture))>
  make_suites(const std::string &name, F &&f) {
    return make_suites<Fixture...>(name, auto_factory, std::forward<F>(f));
  }

  template<typename ...T>
  using suite = basic_suite<expectation_error, T...>;

} // namespace mettle

#endif
