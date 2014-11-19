#include "forked_test_runner.hpp"

#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <sys/wait.h>

#include <mettle/driver/scoped_pipe.hpp>
#include <mettle/driver/scoped_sig_intr.hpp>
#include <mettle/driver/test_monitor.hpp>
#include "../utils.hpp"

namespace mettle {

namespace {
  inline test_result parent_failed() {
    return { false, err_string(errno) };
  }

  [[noreturn]] inline void child_failed() {
    _exit(128);
  }
}

test_result forked_test_runner::operator ()(
  const test_function &test, log::test_output &output
) const {
  scoped_pipe stdout_pipe, stderr_pipe, log_pipe;
  if(stdout_pipe.open() < 0 ||
     stderr_pipe.open() < 0 ||
     log_pipe.open(O_CLOEXEC) < 0)
    return parent_failed();

  fflush(nullptr);

  scoped_sig_intr intr;
  intr.open(SIGCHLD);

  pid_t pid;
  if((pid = fork()) < 0)
    return parent_failed();

  if(pid == 0) {
    intr.close();

    // Make a new process group so we can kill the test and all its children
    // as a group.
    setpgid(0, 0);

    if(timeout_)
      fork_monitor(*timeout_);

    if(stdout_pipe.close_read() < 0 ||
       stderr_pipe.close_read() < 0 ||
       log_pipe.close_read() < 0)
      child_failed();

    if(stdout_pipe.move_write(STDOUT_FILENO) < 0 ||
       stderr_pipe.move_write(STDERR_FILENO) < 0)
      child_failed();

    auto result = test();
    if(write(log_pipe.write_fd, result.message.c_str(),
             result.message.length()) < 0)
      child_failed();

    fflush(nullptr);

    _exit(!result.passed);
  }
  else {
    if(stdout_pipe.close_write() < 0 ||
       stderr_pipe.close_write() < 0 ||
       log_pipe.close_write() < 0)
      return parent_failed();

    std::string message;
    std::vector<readfd> dests = {
      {stdout_pipe.read_fd, &output.stdout},
      {stderr_pipe.read_fd, &output.stderr},
      {log_pipe.read_fd,    &message}
    };

    // Read from the piped stdout, stderr, and log. If we're interrupted
    // (probably by SIGCHLD), do one last non-blocking read to get any data we
    // might have missed.
    sigset_t empty;
    sigemptyset(&empty);
    if(read_into(dests, nullptr, &empty) < 0) {
      if(errno != EINTR)
        return parent_failed();
      timespec timeout = {0, 0};
      if(read_into(dests, &timeout, nullptr) < 0)
        return parent_failed();
    }

    int status;
    if(waitpid(pid, &status, 0) < 0)
      return parent_failed();

    // Make sure everything in the test's process group is dead. Don't worry
    // about reaping.
    kill(-pid, SIGKILL);

    if(WIFEXITED(status)) {
      int exit_code = WEXITSTATUS(status);
      if(exit_code == err_timeout) {
        std::ostringstream ss;
        ss << "Timed out after " << timeout_->count() << " ms";
        return { false, ss.str() };
      }
      else {
        return { exit_code == 0, message };
      }
    }
    else { // WIFSIGNALED
      return { false, strsignal(WTERMSIG(status)) };
    }
  }
}

} // namespace mettle
