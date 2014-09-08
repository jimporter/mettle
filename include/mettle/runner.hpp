#ifndef INC_METTLE_RUNNER_HPP
#define INC_METTLE_RUNNER_HPP

#include "compiled_suite.hpp"
#include "log/core.hpp"

namespace mettle {

using test_runner = std::function<
  test_result(const test_function &, log::test_output &)
>;

namespace detail {
  class suite_stack {
  public:
    using value_type = std::vector<std::string>;

    template<typename T>
    void push(T &&t) {
      queued_.push_back(std::forward<T>(t));
    }

    void pop() {
      if(!queued_.empty())
        queued_.pop_back();
      else
        committed_.pop_back();
    }

    template<typename T>
    void commit(T &&callback) {
      if(!queued_.empty()) {
        for(auto &&i : queued_) {
          committed_.push_back(std::move(i));
          callback(committed_);
        }
        queued_.clear();
      }
    }

    const value_type committed() const {
      return committed_;
    }

    bool has_queued() const {
      return !queued_.empty();
    }
  private:
    value_type committed_, queued_;
  };

  template<typename Suites, typename Filter>
  void run_tests_impl(
    const Suites &suites, log::test_logger &logger, const test_runner &runner,
    const Filter &filter, suite_stack &parents
  ) {
    for(const auto &suite : suites) {
      parents.push(suite.name());

      for(const auto &test : suite) {
        auto action = filter(test.attrs);

        if(action.first == attr_action::hide)
          continue;
        parents.commit([&logger](const auto &committed) {
          logger.started_suite(committed);
        });

        const log::test_name name = {parents.committed(), test.name, test.id};
        logger.started_test(name);

        if(action.first == attr_action::skip) {
          logger.skipped_test(name, action.second);
          continue;
        }

        log::test_output output;
        auto result = runner(test.function, output);
        if(result.passed)
          logger.passed_test(name, output);
        else
          logger.failed_test(name, result.message, output);
      }

      run_tests_impl(suite.subsuites(), logger, runner, filter, parents);

      if(!parents.has_queued())
        logger.ended_suite(parents.committed());
      parents.pop();
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
  detail::suite_stack parents;
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
