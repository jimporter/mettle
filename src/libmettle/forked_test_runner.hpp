#ifndef INC_METTLE_SRC_LIBMETTLE_FORKED_TEST_RUNNER_HPP
#define INC_METTLE_SRC_LIBMETTLE_FORKED_TEST_RUNNER_HPP

#include <chrono>

#include <mettle/suite/compiled_suite.hpp>
#include <mettle/driver/log/core.hpp>
#include <mettle/driver/detail/optional.hpp>

namespace mettle {

class forked_test_runner {
public:
  using timeout_t = METTLE_OPTIONAL_NS::optional<std::chrono::milliseconds>;

  forked_test_runner(timeout_t timeout = {}) : timeout_(timeout) {}

  template<class Rep, class Period>
  forked_test_runner(std::chrono::duration<Rep, Period> timeout)
    : timeout_(timeout) {}

  test_result
  operator ()(const test_function &test, log::test_output &output) const;
private:
  static void fork_monitor(std::chrono::milliseconds timeout);

  timeout_t timeout_;
};

} // namespace mettle

#endif
