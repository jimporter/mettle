#ifndef INC_METTLE_RUNNER_HPP
#define INC_METTLE_RUNNER_HPP

#include <iostream>
#include <boost/program_options.hpp>

#include "suite.hpp"

namespace mettle {

struct test_name {
  std::vector<std::string> suites;
  std::string test;
  size_t id;

  std::string full_name() const {
    std::stringstream s;
    for(auto &i : suites)
      s << i << " > ";
    s << test;
    return s.str();
  }
};

inline bool operator ==(const test_name &lhs, const test_name &rhs) {
  return lhs.id == rhs.id;
}
inline bool operator !=(const test_name &lhs, const test_name &rhs) {
  return lhs.id != rhs.id;
}
inline bool operator <(const test_name &lhs, const test_name &rhs) {
  return lhs.id < rhs.id;
}
inline bool operator <=(const test_name &lhs, const test_name &rhs) {
  return lhs.id <= rhs.id;
}
inline bool operator >(const test_name &lhs, const test_name &rhs) {
  return lhs.id > rhs.id;
}
inline bool operator >=(const test_name &lhs, const test_name &rhs) {
  return lhs.id >= rhs.id;
}

struct test_results {
  test_results() : passes(0), skips(0), total(0) {}

  struct failure {
    test_name test;
    std::string message;
  };

  std::vector<const failure> failures;
  size_t passes, skips, total;
};

class test_logger {
public:
  virtual ~test_logger() {}

  virtual void start_suite(const std::vector<std::string> &suites) = 0;
  virtual void end_suite(const std::vector<std::string> &suites) = 0;

  virtual void start_test(const test_name &test) = 0;
  virtual void passed_test(const test_name &test) = 0;
  virtual void skipped_test(const test_name &test) = 0;
  virtual void failed_test(const test_name &test,
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
        const test_name name = {parents, test.name, test.id};
        results.total++;

        logger.start_test(name);
        if(test.skip) {
          results.skips++;
          logger.skipped_test(name);
          continue;
        }

        auto result = test.function();
        if(result.passed) {
          results.passes++;
          logger.passed_test(name);
        }
        else {
          results.failures.push_back({name, result.message});
          logger.failed_test(name, result.message);
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
