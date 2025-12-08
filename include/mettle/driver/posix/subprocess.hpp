#ifndef INC_METTLE_DRIVER_POSIX_SUBPROCESS_HPP
#define INC_METTLE_DRIVER_POSIX_SUBPROCESS_HPP

#include <signal.h>

#include <chrono>
#include <string>
#include <vector>

#include "../detail/export.hpp"

namespace mettle::posix {

  struct readfd {
    int fd;
    std::string *dest;
  };

  METTLE_PUBLIC void make_timeout_monitor(std::chrono::milliseconds timeout);

  METTLE_PUBLIC int read_into(std::vector<readfd> &dests,
                              const timespec *timeout, const sigset_t *sigmask);

  METTLE_PUBLIC int send_pgid(int fd, int pgid);
  METTLE_PUBLIC int recv_pgid(int fd, int *pgid);

} // namespace mettle::posix

#endif
