#ifndef INC_METTLE_RUNNER_HPP
#define INC_METTLE_RUNNER_HPP

#include <iostream>
#include <boost/program_options.hpp>

#include "suite.hpp"

namespace mettle {

namespace detail {
  template<typename T>
  std::string join(T begin, T end, const std::string &delim) {
    if(begin == end)
      return "";

    std::stringstream s;
    s << *begin;
    for(++begin; begin != end; ++begin)
      s << delim << *begin;
    return s.str();
  }
}

struct test_results {
  test_results() : passes(0), skips(0), total(0) {}

  struct failure {
    std::vector<std::string> suites;
    std::string test, message;

    std::string full_name() const {
      std::stringstream s;
      s << detail::join(suites.begin(), suites.end(), " > ") << " > " << test;
      return s.str();
    }
  };

  std::vector<failure> failures;
  size_t passes, skips, total;
};

class test_logger {
public:
  virtual ~test_logger() {}

  virtual void start_suite(const std::vector<std::string> &suites) = 0;
  virtual void end_suite(const std::vector<std::string> &suites) = 0;

  virtual void start_test(const std::vector<std::string> &suites,
                          const std::string &test) = 0;
  virtual void passed_test(const std::vector<std::string> &suites,
                           const std::string &test) = 0;
  virtual void skipped_test(const std::vector<std::string> &suites,
                            const std::string &test) = 0;
  virtual void failed_test(const std::vector<std::string> &suites,
                           const std::string &test,
                           const std::string &message) = 0;

  virtual void summarize(const test_results &results) = 0;
};

namespace detail {
  template<typename T>
  void run_tests_impl(
    test_results &results, const T &suites, test_logger &logger,
    std::vector<std::string> &parents
  ) {
    for(auto &suite : suites) {
      parents.push_back(suite.name());

      logger.start_suite(parents);

      for(auto &test : suite) {
        results.total++;

        logger.start_test(parents, test.name);
        if(test.skip) {
          results.skips++;
          logger.skipped_test(parents, test.name);
          continue;
        }

        auto result = test.function();
        if(result.passed) {
          results.passes++;
          logger.passed_test(parents, test.name);
        }
        else {
          results.failures.push_back({parents, test.name, result.message});
          logger.failed_test(parents, test.name, result.message);
        }
      }

      logger.end_suite(parents);

      run_tests_impl(results, suite.subsuites(), logger, parents);
      parents.pop_back();
    }
  }
}

template<typename T>
inline size_t run_tests(const T &suites, test_logger &logger) {
  test_results results;
  std::vector<std::string> parents;
  detail::run_tests_impl(results, suites, logger, parents);
  logger.summarize(results);
  return results.failures.size();
}

template<typename T>
inline size_t run_tests(const T &suites, test_logger &&logger) {
  return run_tests(suites, logger);
}

} // namespace mettle

#endif
