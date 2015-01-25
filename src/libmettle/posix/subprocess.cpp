#include <mettle/driver/posix/subprocess.hpp>

#include <cassert>
#include <thread>

#include <pthread.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>

namespace mettle {

namespace posix {

  namespace {
    int fd_to_close;
    void atfork_close_fd() {
      close(fd_to_close);
    }

    [[noreturn]] void monitor_failed(int pid1 = -1, int pid2 = -1) {
      if(pid1 != -1)
        kill(pid1, SIGKILL);
      if(pid2 != -1)
        kill(pid2, SIGKILL);
      _exit(128);
    }
  }

  void make_timeout_monitor(std::chrono::milliseconds timeout) {
    // Assume our process group is different from our parent's. However, we want
    // some of our processes to rejoin the parent's process group.
    pid_t parent_pgid = getpgid(getppid());
    pid_t test_pgid = getpgid(0);
    assert(parent_pgid != test_pgid);

    pid_t timer_pid;
    if((timer_pid = fork()) < 0)
      monitor_failed();
    if(timer_pid == 0) {
      if(setpgid(0, parent_pgid) < 0)
        monitor_failed();
      std::this_thread::sleep_for(timeout);
      _exit(err_timeout);
    }

    if(setpgid(timer_pid, parent_pgid) < 0)
      monitor_failed(timer_pid);

    // The monitor process will signal the test process when it's ok to proceed
    // (i.e. once it's rejoined the parent's process group). Set up a signal
    // mask to wait for it.
    sigset_t mask, oldmask;
    sigemptyset(&mask);
    sigaddset(&mask, SIGUSR1);
    sigprocmask(SIG_BLOCK, &mask, &oldmask);

    pid_t test_pid;
    if((test_pid = fork()) < 0)
      monitor_failed(timer_pid);
    if(test_pid != 0) {
      sigprocmask(SIG_SETMASK, &oldmask, nullptr);
      if(setpgid(0, parent_pgid) < 0 ||
         kill(test_pid, SIGUSR1) < 0)
        monitor_failed(timer_pid, test_pid);

      // Wait for the first child process (the timer or the test) to finish,
      // then kill (and wait) for the other one. If the timer finishes first,
      // kill the test's entire process group.
      int status;
      pid_t exited_pid;
      if((exited_pid = wait(&status)) < 0)
        monitor_failed(timer_pid, -test_pgid);
      if(kill(exited_pid == test_pid ? timer_pid : -test_pgid, SIGKILL) < 0)
        monitor_failed(timer_pid, -test_pgid);
      wait(nullptr);

      if(WIFEXITED(status))
        _exit(WEXITSTATUS(status));
      else // WIFSIGNALED
        raise(WTERMSIG(status));
    }
    else {
      int sig;
      if(sigwait(&mask, &sig) < 0)
        monitor_failed();
      sigprocmask(SIG_SETMASK, &oldmask, nullptr);
    }
  }

  int read_into(std::vector<readfd> &dests, const timespec *timeout,
                const sigset_t *sigmask) {
    while(true) {
      int maxfd = -1;
      fd_set fds;
      FD_ZERO(&fds);
      for(const auto &i : dests) {
        if(i.fd >= 0) {
          maxfd = std::max(maxfd, i.fd);
          FD_SET(i.fd, &fds);
        }
      }
      if(maxfd < 0)
        return 0;

      int rv = pselect(maxfd + 1, &fds, nullptr, nullptr, timeout, sigmask);
      if(rv <= 0)
        return rv;

      for(auto &i : dests) {
        if(i.fd >= 0 && FD_ISSET(i.fd, &fds)) {
          ssize_t size;
          char buf[BUFSIZ];

          if((size = read(i.fd, buf, sizeof(buf))) < 0)
            return size;
          if(size == 0)
            i.fd = -i.fd;
          else
            i.dest->append(buf, size);
        }
      }
    }
  }

  int make_fd_private(int fd) {
    fd_to_close = fd;
    return pthread_atfork(nullptr, nullptr, atfork_close_fd);
  }

}

} // namespace mettle
