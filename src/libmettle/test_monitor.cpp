#include <mettle/driver/test_monitor.hpp>

#include <signal.h>
#include <sys/wait.h>
#include <unistd.h>

#include <thread>

namespace mettle {

namespace {
  [[noreturn]] inline void child_failed() {
    _exit(128);
  }
}

void fork_monitor(std::chrono::milliseconds timeout) {
  pid_t timer_pid;
  if((timer_pid = fork()) < 0)
    child_failed();
  if(timer_pid == 0) {
    std::this_thread::sleep_for(timeout);
    _exit(err_timeout);
  }

  // The test process will signal us when it's ok to proceed (i.e. once it's
  // created a new process group). Set up a signal mask to wait for it.
  sigset_t mask, oldmask;
  sigemptyset(&mask);
  sigaddset(&mask, SIGUSR1);
  sigprocmask(SIG_BLOCK, &mask, &oldmask);

  pid_t test_pid;
  if((test_pid = fork()) < 0) {
    kill(timer_pid, SIGKILL);
    child_failed();
  }
  if(test_pid != 0) {
    // Wait for the first child process (the timer or the test) to finish,
    // then kill and wait for the other one.
    int status;
    pid_t exited_pid = wait(&status);
    if(exited_pid == test_pid) {
      kill(timer_pid, SIGKILL);
    }
    else {
      // Wait until the test process has created its process group.
      int sig;
      sigwait(&mask, &sig);
      kill(-test_pid, SIGKILL);
    }
    wait(nullptr);

    if(WIFEXITED(status)) {
      _exit(WEXITSTATUS(status));
    }
    else if(WIFSIGNALED(status)) {
      raise(WTERMSIG(status));
    }
    else { // WIFSTOPPED
      kill(exited_pid, SIGKILL);
      raise(WSTOPSIG(status));
    }
  }
  else {
    sigprocmask(SIG_SETMASK, &oldmask, nullptr);
  }
}

void notify_monitor() {
  kill(getppid(), SIGUSR1);
}

} // namespace mettle
