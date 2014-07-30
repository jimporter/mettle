#ifndef INC_METTLE_RUNNER_HPP
#define INC_METTLE_RUNNER_HPP

#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>

#include <chrono>
#include <thread>

#include "optional.hpp"
#include "scoped_pipe.hpp"
#include "suite.hpp"
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

      logger.start_suite(parents);

      for(const auto &test : suite) {
        const log::test_name name = {parents, test.name, test.id};
        logger.start_test(name);

        if(test.skip) {
          logger.skipped_test(name);
          continue;
        }

        log::test_output output;
        auto result = runner(test.function, output);
        if(result.passed)
          logger.passed_test(name, output);
        else
          logger.failed_test(name, result.message, output);
      }

      logger.end_suite(parents);

      run_tests_impl(suite.subsuites(), logger, runner, parents);
      parents.pop_back();
    }
  }
}

class forked_test_runner {
private:
  using timeout_t = METTLE_OPTIONAL_NS::optional<std::chrono::milliseconds>;
public:
  forked_test_runner(timeout_t timeout = timeout_t()) : timeout_(timeout) {}

  template<class Rep, class Period>
  forked_test_runner(std::chrono::duration<Rep, Period> timeout)
    : timeout_(timeout) {}

  test_result
  operator ()(const test_function &test, log::test_output &output) const {
    scoped_pipe stdout_pipe, stderr_pipe, log_pipe;
    if(stdout_pipe.open() < 0 ||
       stderr_pipe.open() < 0 ||
       log_pipe.open(O_CLOEXEC) < 0)
      goto parent_fail;

    fflush(nullptr);

    pid_t pid;
    if((pid = fork()) < 0)
      goto parent_fail;

    if(pid == 0) {
      if(timeout_)
        fork_watcher(*timeout_);

      if(stdout_pipe.close_read() < 0 ||
         stderr_pipe.close_read() < 0 ||
         log_pipe.close_read() < 0)
        goto child_fail;

      if(dup2(stdout_pipe.write_fd, STDOUT_FILENO) < 0 ||
         dup2(stderr_pipe.write_fd, STDERR_FILENO) < 0)
        goto child_fail;

      if(stdout_pipe.close_write() < 0 ||
         stderr_pipe.close_write() < 0)
        goto child_fail;

      {
        auto result = test();
        if(write(log_pipe.write_fd, result.message.c_str(),
                 result.message.length()) < 0)
          goto child_fail;

        fflush(nullptr);
        _exit(!result.passed);
      }
    child_fail:
      _exit(128);
    }
    else {
      if(stdout_pipe.close_write() < 0 ||
         stderr_pipe.close_write() < 0 ||
         log_pipe.close_write() < 0)
        goto parent_fail;

      ssize_t size;
      char buf[BUFSIZ];

      // Read from the piped stdout and stderr.
      int rv;
      pollfd fds[2] = { {stdout_pipe.read_fd, POLLIN, 0},
                        {stderr_pipe.read_fd, POLLIN, 0} };
      std::string *dests[] = {&output.stdout, &output.stderr};
      int open_fds = 2;
      while(open_fds && (rv = poll(fds, 2, -1)) > 0) {
        for(size_t i = 0; i < 2; i++) {
          if(fds[i].revents & POLLIN) {
            if((size = read(fds[i].fd, buf, sizeof(buf))) < 0)
              goto parent_fail;
            dests[i]->append(buf, size);
          }
          if(fds[i].revents & POLLHUP) {
            fds[i].fd = -fds[i].fd;
            open_fds--;
          }
        }
      }
      if(rv < 0) // poll() failed!
        goto parent_fail;

      // Read from our logging pipe (which sends the message from the test run).
      std::string message;
      while((size = read(log_pipe.read_fd, buf, sizeof(buf))) > 0)
        message.append(buf, size);
      if(size < 0) // read() failed!
        goto parent_fail;

      int status;
      if(waitpid(pid, &status, 0) < 0)
        goto parent_fail;

      if(WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        if(exit_code == 2) {
          std::stringstream ss;
          ss << "Timed out after " << timeout_->count() << " ms";
          return { false, ss.str() };
        }
        else {
          return { exit_code == 0, message };
        }
      }
      else if(WIFSIGNALED(status)) {
        return { false, strsignal(WTERMSIG(status)) };
      }
      else { // WIFSTOPPED
        return { false, "Stopped" };
      }
    }
  parent_fail:
    char errbuf[256];
    strerror_r(errno, errbuf, sizeof(errbuf));
    return { false, errbuf };
  }
private:
  void fork_watcher(std::chrono::milliseconds timeout) const {
    pid_t watcher_pid;
    if((watcher_pid = fork()) < 0)
      goto fail;
    if(watcher_pid == 0) {
      std::this_thread::sleep_for(timeout);
      _exit(2);
    }

    pid_t test_pid;
    if((test_pid = fork()) < 0) {
      kill(watcher_pid, SIGKILL);
      goto fail;
    }
    if(test_pid != 0) {
      // Wait for the first child process (the watcher or the test) to finish,
      // and kill the other one.
      int status;
      pid_t exited_pid = wait(&status);
      kill(exited_pid == test_pid ? watcher_pid : test_pid, SIGKILL);
      wait(nullptr);

      if(WIFEXITED(status))
        exit(WEXITSTATUS(status));
      else if(WIFSIGNALED(status))
        raise(WTERMSIG(status));
      else // WIFSTOPPED
        _exit(128); // XXX: not sure what to do here
    }

    return;
  fail:
    _exit(128);
  }

  timeout_t timeout_;
};

inline test_result
inline_test_runner(const test_function &test, log::test_output &) {
  return test();
}

template<typename T>
inline void run_tests(const T &suites, log::test_logger &logger,
                      const test_runner &runner) {
  std::vector<std::string> parents;
  logger.start_run();
  detail::run_tests_impl(suites, logger, runner, parents);
  logger.end_run();
}

template<typename T>
inline void run_tests(const T &suites, log::test_logger &&logger,
                      const test_runner &runner) {
  run_tests(suites, logger, runner);
}

} // namespace mettle

#endif
