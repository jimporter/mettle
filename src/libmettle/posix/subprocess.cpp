#include <mettle/driver/posix/subprocess.hpp>

#include <cassert>
#include <initializer_list>
#include <thread>

#include <pthread.h>
#include <signal.h>
#include <sys/select.h>
#include <sys/wait.h>
#include <unistd.h>

#include <mettle/driver/exit_code.hpp>

namespace mettle::posix {

  namespace {
    [[noreturn]] void monitor_failed(std::initializer_list<int> pids = {}) {
      for(int pid : pids)
        kill(pid, SIGKILL);
      _exit(exit_code::fatal);
    }

    inline int size_to_status(int size) {
      if(size < 0)
        return size;
      if(size == sizeof(int))
        return 0;
      errno = EIO;
      return -1;
    }
  }

  void make_timeout_monitor(std::chrono::milliseconds timeout) {
    pid_t timer_pid;
    if((timer_pid = fork()) < 0)
      monitor_failed();
    if(timer_pid == 0) {
      std::this_thread::sleep_for(timeout);
      _exit(exit_code::timeout);
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
    return size_to_status( write(fd, &pgid, sizeof(pgid)) );
  }

  int recv_pgid(int fd, int *pgid) {
    return size_to_status( read(fd, pgid, sizeof(*pgid)) );
  }

} // namespace mettle::posix
