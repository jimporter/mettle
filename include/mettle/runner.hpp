#ifndef INC_METTLE_RUNNER_HPP
#define INC_METTLE_RUNNER_HPP

#include "compiled_suite.hpp"
#include "log/core.hpp"

namespace mettle {

using test_runner = std::function<
  test_result(const test_function &, log::test_output &)
>;

namespace detail {
  template<typename Suites, typename Filter>
  void run_tests_impl(const Suites &suites, log::test_logger &logger,
                      const test_runner &runner, const Filter &filter,
                      std::vector<std::string> &parents) {
    for(const auto &suite : suites) {
      parents.push_back(suite.name());

      // XXX: Don't emit started/ended_suite if all the children are hidden.
      logger.started_suite(parents);

      for(const auto &test : suite) {
        auto action = filter(test.attrs);

        if(action.first == attr_action::hide)
          continue;

        const log::test_name name = {parents, test.name, test.id};
        logger.started_test(name);

        if(action.first == attr_action::skip) {
          auto &&attr = *action.second;
          logger.skipped_test(
            name, attr.value.empty() ? "" : *attr.value.begin()
          );
          continue;
        }

        log::test_output output;
        auto result = runner(test.function, output);
        if(result.passed)
          logger.passed_test(name, output);
        else
          logger.failed_test(name, result.message, output);
      }

      logger.ended_suite(parents);

      run_tests_impl(suite.subsuites(), logger, runner, filter, parents);
      parents.pop_back();
    }
  }
}

inline test_result
inline_test_runner(const test_function &test, log::test_output &) {
  return test();
}

template<typename Suites, typename Filter>
void run_tests(const Suites &suites, log::test_logger &logger,
               const test_runner &runner, const Filter &filter) {
  std::vector<std::string> parents;
  logger.started_run();
  detail::run_tests_impl(suites, logger, runner, filter, parents);
  logger.ended_run();
}

template<typename Suites, typename Filter>
inline void run_tests(const Suites &suites, log::test_logger &&logger,
                      const test_runner &runner, const Filter &filter) {
  run_tests(suites, logger, runner, filter);
}

template<typename Suites>
inline void run_tests(const Suites &suites, log::test_logger &logger,
                      const test_runner &runner) {
  run_tests(suites, logger, runner, default_attr_filter());
}

template<typename Suites>
inline void run_tests(const Suites &suites, log::test_logger &&logger,
                      const test_runner &runner) {
  run_tests(suites, logger, runner, default_attr_filter());
}


} // namespace mettle

#endif
