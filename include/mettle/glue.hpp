#ifndef INC_METTLE_GLUE_HPP
#define INC_METTLE_GLUE_HPP

#include "suite/make_suite.hpp"
#include "suite/global_suite.hpp"
#include "matchers/expect.hpp"

namespace mettle {

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
  return make_suite<Fixture...>(name, attrs, auto_factory, std::forward<F>(f));
}

template<typename ...Fixture, typename F>
inline runnable_suite
make_suite(const std::string &name, F &&f) {
  return make_suite<Fixture...>(name, auto_factory, std::forward<F>(f));
}

template<typename ...Fixture, typename Factory, typename F>
inline std::array<runnable_suite, std::max<size_t>(sizeof...(Fixture), 1)>
make_suites(const std::string &name, const attributes &attrs,
           Factory &&factory, F &&f) {
  return make_basic_suites<expectation_error, Fixture...>(
    name, attrs, std::forward<Factory>(factory), std::forward<F>(f)
  );
}

template<typename ...Fixture, typename Factory, typename F>
inline std::array<runnable_suite, std::max<size_t>(sizeof...(Fixture), 1)>
make_suites(const std::string &name, Factory &&factory, F &&f) {
  return make_basic_suites<expectation_error, Fixture...>(
    name, std::forward<Factory>(factory), std::forward<F>(f)
  );
}

template<typename ...Fixture, typename F>
inline std::array<runnable_suite, std::max<size_t>(sizeof...(Fixture), 1)>
make_suites(const std::string &name, const attributes &attrs, F &&f) {
  return make_suites<Fixture...>(name, attrs, auto_factory, std::forward<F>(f));
}

template<typename ...Fixture, typename F>
inline std::array<runnable_suite, std::max<size_t>(sizeof...(Fixture), 1)>
make_suites(const std::string &name, F &&f) {
  return make_suites<Fixture...>(name, auto_factory, std::forward<F>(f));
}

template<typename ...T>
using suite = basic_suite<expectation_error, T...>;

} // namespace mettle

#endif
