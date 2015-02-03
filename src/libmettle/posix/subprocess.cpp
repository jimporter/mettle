#include <mettle/driver/posix/subprocess.hpp>

#include <cassert>
#include <initializer_list>
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

    [[noreturn]] void monitor_failed(std::initializer_list<int> pids = {}) {
      for(int pid : pids)
        kill(pid, SIGKILL);
      _exit(128);
    }
  }

  void make_timeout_monitor(std::chrono::milliseconds timeout) {
    pid_t timer_pid;
    if((timer_pid = fork()) < 0)
      monitor_failed();
    if(timer_pid == 0) {
      std::this_thread::sleep_for(timeout);
      _exit(err_timeout);
    }

    pid_t test_pid;
    if((test_pid = fork()) < 0)
      monitor_failed({timer_pid});
    if(test_pid != 0) {
      // Wait for the first child process (the timer or the test) to finish,
      // then kill (and wait) for the other one. If the timer finishes first,
      // kill the test's entire process group.
      int status;
      pid_t exited_pid;
      if((exited_pid = wait(&status)) < 0)
        monitor_failed({timer_pid, test_pid});
      if(kill(exited_pid == test_pid ? timer_pid : test_pid, SIGKILL) < 0)
        monitor_failed({timer_pid, test_pid});
      wait(nullptr);

      if(WIFEXITED(status))
        _exit(WEXITSTATUS(status));
      else // WIFSIGNALED
        raise(WTERMSIG(status));
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

  int send_pgid(int fd, int pgid) {
    return write(fd, &pgid, sizeof(pgid));
  }

  int recv_pgid(int fd, int *pgid) {
    return read(fd, pgid, sizeof(*pgid));
  }

  int make_fd_private(int fd) {
    fd_to_close = fd;
    return pthread_atfork(nullptr, nullptr, atfork_close_fd);
  }

}

} // namespace mettle
