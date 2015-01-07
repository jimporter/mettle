#ifndef INC_METTLE_DRIVER_TEST_MONITOR_HPP
#define INC_METTLE_DRIVER_TEST_MONITOR_HPP

#include <signal.h>

#include <chrono>
#include <string>
#include <vector>

namespace mettle {

constexpr const int err_timeout = 64;

struct readfd {
  int fd;
  std::string *dest;
};

void fork_monitor(std::chrono::milliseconds timeout);
int read_into(std::vector<readfd> &dests, const timespec *timeout,
              const sigset_t *sigmask);

} // namespace mettle

#endif
