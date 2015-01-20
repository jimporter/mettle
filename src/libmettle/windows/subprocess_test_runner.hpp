#ifndef INC_METTLE_SRC_LIBMETTLE_WINDOWS_SUBPROCESS_TEST_RUNNER_HPP
#define INC_METTLE_SRC_LIBMETTLE_WINDOWS_SUBPROCESS_TEST_RUNNER_HPP

#include <chrono>

#include <wtypes.h>

#include <mettle/suite/compiled_suite.hpp>
#include <mettle/driver/log/core.hpp>
#include <mettle/driver/detail/optional.hpp>

namespace mettle {

namespace windows {

  class subprocess_test_runner {
  public:
    using timeout_t = METTLE_OPTIONAL_NS::optional<std::chrono::milliseconds>;

    subprocess_test_runner(timeout_t timeout = {}) : timeout_(timeout) {}

    template<class Rep, class Period>
    subprocess_test_runner(std::chrono::duration<Rep, Period> timeout)
      : timeout_(timeout) {}

    test_result
    operator ()(const test_info &test, log::test_output &output) const;
  private:
    timeout_t timeout_;
  };

  const test_info * find_test(const suites_list &suites, test_uid id);
  bool run_single_test(const test_info &test, HANDLE log_pipe);

}

} // namespace mettle

#endif
