#ifndef INC_METTLE_DRIVER_TEST_MONITOR_HPP
#define INC_METTLE_DRIVER_TEST_MONITOR_HPP

#include <chrono>

namespace mettle {

constexpr int err_timeout = 64;

void fork_monitor(std::chrono::milliseconds timeout);
void notify_monitor();

} // namespace mettle

#endif
