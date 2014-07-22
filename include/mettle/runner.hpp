#ifndef INC_METTLE_RUNNER_HPP
#define INC_METTLE_RUNNER_HPP

#include <fcntl.h>
#include <poll.h>
#include <sys/wait.h>
#include <unistd.h>

#include "suite.hpp"
#include "log/core.hpp"

namespace mettle {

namespace detail {
  struct scoped_pipe {
    scoped_pipe() : read_fd(-1), write_fd(-1) {}

    int open(int flags = 0) {
      return pipe2(&read_fd, flags);
    }

    ~scoped_pipe() {
      close_read();
      close_write();
    }

    int close_read() {
      return do_close(read_fd);
    }

    int close_write() {
      return do_close(write_fd);
    }

    int read_fd, write_fd;
  private:
    int do_close(int &fd) {
      if(fd == -1)
        return 0;
      int err = ::close(fd);
      if(err == 0)
        fd = -1;
      return err;
    }
  };

  inline test_result run_test(const std::function<test_result(void)> &test,
                              log::test_output &output) {
    scoped_pipe stdout_pipe, stderr_pipe, log_pipe;
    if(stdout_pipe.open() < 0 ||
       stderr_pipe.open() < 0 ||
       log_pipe.open(O_CLOEXEC) < 0)
      goto parent_fail;

    pid_t pid;
    if((pid = fork()) < 0)
      goto parent_fail;

    if(pid == 0) {
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
        exit(result.passed ? 0 : 1);
      }
    child_fail:
      exit(1); // XXX: Pass the errno somehow?
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
      int open_fds = 2;
      while(open_fds && (rv = poll(fds, 2, -1)) > 0) {
        for(size_t i = 0; i < 2; i++) {
          auto &stream = i == 0 ? output.stdout : output.stderr;
          if(fds[i].revents & POLLIN) {
            if((size = read(fds[i].fd, buf, sizeof(buf))) < 0)
              goto parent_fail;
            stream.write(buf, size);
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
      std::stringstream message;
      while((size = read(log_pipe.read_fd, buf, sizeof(buf))) > 0)
        message.write(buf, size);
      if(size < 0) // read() failed!
        goto parent_fail;

      int status;
      if(waitpid(pid, &status, 0) < 0)
        goto parent_fail;

      if(WIFSIGNALED(status))
        return { false, strsignal(WTERMSIG(status)) };

      return { WIFEXITED(status) && WEXITSTATUS(status) == 0, message.str() };
    }
  parent_fail:
    char errbuf[256];
    strerror_r(errno, errbuf, sizeof(errbuf));
    return { false, errbuf };
  }

  template<typename T>
  void run_tests_impl(const T &suites, log::test_logger &logger,
                      bool fork_tests, std::vector<std::string> &parents) {
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
        auto result = fork_tests ?
          run_test(test.function, output) : test.function();
        if(result.passed)
          logger.passed_test(name, output);
        else
          logger.failed_test(name, result.message, output);
      }

      logger.end_suite(parents);

      run_tests_impl(suite.subsuites(), logger, fork_tests, parents);
      parents.pop_back();
    }
  }
}

template<typename T>
inline void run_tests(const T &suites, log::test_logger &logger,
                      bool fork_tests = true) {
  std::vector<std::string> parents;
  logger.start_run();
  detail::run_tests_impl(suites, logger, fork_tests, parents);
  logger.end_run();
}

template<typename T>
inline void run_tests(const T &suites, log::test_logger &&logger,
                      bool fork_tests = true) {
  run_tests(suites, logger, fork_tests);
}

} // namespace mettle

#endif
