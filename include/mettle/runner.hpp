#ifndef INC_METTLE_RUNNER_HPP
#define INC_METTLE_RUNNER_HPP

#include "compiled_suite.hpp"
#include "log/core.hpp"

namespace mettle {

using test_runner = std::function<
  test_result(const test_function &, log::test_output &)
>;

namespace detail {
  template<typename T>
  void run_tests_impl(const T &suites, log::test_logger &logger,
                      const test_runner &runner,
                      std::vector<std::string> &parents) {
    for(const auto &suite : suites) {
      parents.push_back(suite.name());

      logger.started_suite(parents);

      for(const auto &test : suite) {
        const log::test_name name = {parents, test.name, test.id};
        logger.started_test(name);

        bool skipped = false;
        for(const auto &attr : test.attrs) {
          if(attr.action() == attr_action::skip) {
            logger.skipped_test(name, attr.value());
            skipped = true;
            break;
          }
        }
        if(skipped)
          continue;

        log::test_output output;
        auto result = runner(test.function, output);
        if(result.passed)
          logger.passed_test(name, output);
        else
          logger.failed_test(name, result.message, output);
      }

      logger.ended_suite(parents);

      run_tests_impl(suite.subsuites(), logger, runner, parents);
      parents.pop_back();
    }
  }
}

inline test_result
inline_test_runner(const test_function &test, log::test_output &) {
  return test();
}

template<typename T>
void run_tests(const T &suites, log::test_logger &logger,
               const test_runner &runner) {
  std::vector<std::string> parents;
  logger.started_run();
  detail::run_tests_impl(suites, logger, runner, parents);
  logger.ended_run();
}

template<typename T>
inline void run_tests(const T &suites, log::test_logger &&logger,
                      const test_runner &runner) {
  run_tests(suites, logger, runner);
}

} // namespace mettle

#endif
