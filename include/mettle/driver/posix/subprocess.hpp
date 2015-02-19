#ifndef INC_METTLE_DRIVER_POSIX_SUBPROCESS_HPP
#define INC_METTLE_DRIVER_POSIX_SUBPROCESS_HPP

#include <signal.h>

#include <chrono>
#include <string>
#include <vector>

namespace mettle {

namespace posix {
  struct readfd {
    int fd;
    std::string *dest;
  };

  void make_timeout_monitor(std::chrono::milliseconds timeout);

  int read_into(std::vector<readfd> &dests, const timespec *timeout,
                const sigset_t *sigmask);

  int send_pgid(int fd, int pgid);
  int recv_pgid(int fd, int *pgid);

  int make_fd_private(int fd);
}

} // namespace mettle

#endif
