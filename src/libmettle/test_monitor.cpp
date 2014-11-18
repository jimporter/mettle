#include <mettle/driver/test_monitor.hpp>

#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <cassert>
#include <thread>

namespace mettle {

namespace {
  [[noreturn]] inline void child_failed() {
    _exit(128);
  }
}

void fork_monitor(std::chrono::milliseconds timeout) {
  // Assume our process group is different from our parent's. However, we want
  // some of our processes to rejoin the parent's process group.
  pid_t parent_pgid = getpgid(getppid());
  assert(parent_pgid != getpgid(0));

  pid_t timer_pid;
  if((timer_pid = fork()) < 0)
    child_failed();
  if(timer_pid == 0) {
    setpgid(0, parent_pgid);
    std::this_thread::sleep_for(timeout);
    _exit(err_timeout);
  }

  pid_t test_pid;
  if((test_pid = fork()) < 0) {
    kill(timer_pid, SIGKILL);
    child_failed();
  }
  if(test_pid != 0) {
    pid_t test_pgid = getpgid(0);
    setpgid(0, parent_pgid);

    // Wait for the first child process (the timer or the test) to finish,
    // then kill (and wait) for the other one. If the timer finishes first, kill
    // the test's entire process group.
    int status;
    pid_t exited_pid = wait(&status);
    kill(exited_pid == test_pid ? timer_pid : -test_pgid, SIGKILL);
    wait(nullptr);

    if(WIFEXITED(status))
      _exit(WEXITSTATUS(status));
    else // WIFSIGNALED
      raise(WTERMSIG(status));
  }
}

} // namespace mettle
