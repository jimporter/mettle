#ifndef INC_METTLE_RUNNER_HPP
#define INC_METTLE_RUNNER_HPP

#include <sys/wait.h>
#include <unistd.h>

#include <iostream>

#include "suite.hpp"

namespace mettle {

struct test_name {
  std::vector<std::string> suites;
  std::string test;
  size_t id;

  std::string full_name() const {
    std::stringstream s;
    for(const auto &i : suites)
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
  inline test_result run_test(const std::function<test_result(void)> &test) {
    int pipefd[2];
    if(pipe(pipefd) < 0)
      throw std::system_error(errno, std::generic_category());

    pid_t pid;
    if((pid = fork()) < 0)
      throw std::system_error(errno, std::generic_category());

    if(pid == 0) {
      close(pipefd[0]);
      auto result = test();
      if(write(pipefd[1], result.message.c_str(), result.message.length()) < 0)
        exit(1); // XXX: Pass the errno somehow?
      close(pipefd[1]);
      exit(result.passed ? 0 : 1);
    }
    else {
      close(pipefd[1]);

      std::stringstream message;
      ssize_t size;
      char buf[BUFSIZ];
      while((size = read(pipefd[0], buf, sizeof(buf))) > 0)
        message.write(buf, size);
      close(pipefd[0]);

      if(size < 0) { // read() failed!
        char err[256] = "";
        strerror_r(errno, err, sizeof(err));
        return { false, err };
      }

      int status;
      if(waitpid(pid, &status, 0) < 0) {
        char err[256] = "";
        strerror_r(errno, err, sizeof(err));
        return { false, err };
      }

      if(WIFSIGNALED(status))
        return { false, strsignal(WTERMSIG(status)) };

      return { WIFEXITED(status) && WEXITSTATUS(status) == 0, message.str() };
    }
  }

  template<typename T>
  void run_tests_impl(const T &suites, test_logger &logger, bool fork_tests,
                      std::vector<std::string> &parents) {
    for(const auto &suite : suites) {
      parents.push_back(suite.name());

      logger.start_suite(parents);

      for(const auto &test : suite) {
        const test_name name = {parents, test.name, test.id};
        logger.start_test(name);

        if(test.skip) {
          logger.skipped_test(name);
          continue;
        }

        auto result = fork_tests ? run_test(test.function) : test.function();
        if(result.passed)
          logger.passed_test(name);
        else
          logger.failed_test(name, result.message);
      }

      logger.end_suite(parents);

      run_tests_impl(suite.subsuites(), logger, fork_tests, parents);
      parents.pop_back();
    }
  }
}

template<typename T>
inline void run_tests(const T &suites, test_logger &logger,
                      bool fork_tests = true) {
  std::vector<std::string> parents;
  logger.start_run();
  detail::run_tests_impl(suites, logger, fork_tests, parents);
  logger.end_run();
}

template<typename T>
inline void run_tests(const T &suites, test_logger &&logger,
                      bool fork_tests = true) {
  run_tests(suites, logger, fork_tests);
}

} // namespace mettle

#endif
