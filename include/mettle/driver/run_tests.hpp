#ifndef INC_METTLE_DRIVER_RUN_TESTS_HPP
#define INC_METTLE_DRIVER_RUN_TESTS_HPP

#include "../suite/compiled_suite.hpp"
#include "filters_core.hpp"
#include "log/core.hpp"

namespace mettle {

  using test_runner = std::function<
    test_result(const test_info &, log::test_output &)
  >;

  namespace detail {

    class suite_stack {
    public:
      using value_type = std::vector<suite_name>;

      template<typename ...T>
      void push(T &&...t) {
        queued_.emplace_back(std::forward<T>(t)...);
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

      value_type all() const {
        value_type all = committed_;
        all.insert(all.end(), queued_.begin(), queued_.end());
        return all;
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
        parents.push(suite.name(), suite.location().file_name(),
                     suite.location().line());

        for(const auto &test : suite.tests()) {
          const test_name name = {
            test.id, parents.all(), test.name, test.location.file_name(),
            test.location.line()
          };
          auto action = filter(name, test.attrs);
          if(action.action == test_action::indeterminate)
            action = filter_by_attr(test.attrs);

          if(action.action == test_action::hide)
            continue;
          parents.commit([&logger](const auto &committed) {
            logger.started_suite(committed);
          });

          logger.started_test(name);

          if(action.action == test_action::skip) {
            logger.skipped_test(name, action.message);
            continue;
          }

          log::test_output output;

          using namespace std::chrono;
          auto then = steady_clock::now();
          auto failed = runner(test, output);
          auto now = steady_clock::now();
          auto duration = duration_cast<log::test_duration>(now - then);

          if(failed)
            logger.failed_test(name, *failed, output, duration);
          else
            logger.passed_test(name, output, duration);
        }

        run_tests_impl(suite.subsuites(), logger, runner, filter, parents);

        if(!parents.has_queued())
          logger.ended_suite(parents.committed());
        parents.pop();
      }
    }

  } // namespace detail

  inline test_result
  inline_test_runner(const test_info &test, log::test_output &) {
    return test.function();
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
    run_tests(suites, logger, runner, default_filter());
  }

  template<typename Suites>
  inline void run_tests(const Suites &suites, log::test_logger &&logger,
                        const test_runner &runner) {
    run_tests(suites, logger, runner, default_filter());
  }

} // namespace mettle

#endif
