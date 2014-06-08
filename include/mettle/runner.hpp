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

class test_logger {
public:
  virtual ~test_logger() {}

  virtual void start_run() = 0;
  virtual void end_run() = 0;

  virtual void start_suite(const std::vector<std::string> &suites) = 0;
  virtual void end_suite(const std::vector<std::string> &suites) = 0;

  virtual void start_test(const test_name &test) = 0;
  virtual void passed_test(const test_name &test) = 0;
  virtual void skipped_test(const test_name &test) = 0;
  virtual void failed_test(const test_name &test,
                           const std::string &message) = 0;
};

namespace detail {
  template<typename T>
  void run_tests_impl(const T &suites, test_logger &logger,
                      std::vector<std::string> &parents) {
    for(auto &suite : suites) {
      parents.push_back(suite.name());

      logger.start_suite(parents);

      for(auto &test : suite) {
        const test_name name = {parents, test.name, test.id};
        logger.start_test(name);

        if(test.skip) {
          logger.skipped_test(name);
          continue;
        }

        auto result = test.function();
        if(result.passed)
          logger.passed_test(name);
        else
          logger.failed_test(name, result.message);
      }

      logger.end_suite(parents);

      run_tests_impl(suite.subsuites(), logger, parents);
      parents.pop_back();
    }
  }
}

template<typename T>
inline void run_tests(const T &suites, test_logger &logger) {
  std::vector<std::string> parents;
  logger.start_run();
  detail::run_tests_impl(suites, logger, parents);
  logger.end_run();
}

template<typename T>
inline void run_tests(const T &suites, test_logger &&logger) {
  run_tests(suites, logger);
}

} // namespace mettle

#endif
