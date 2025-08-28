#ifndef INC_METTLE_DRIVER_SUBPROCESS_TEST_RUNNER_HPP
#define INC_METTLE_DRIVER_SUBPROCESS_TEST_RUNNER_HPP

#include <chrono>
#include <optional>

#ifdef _WIN32
#  include <wtypes.h>
#endif

#include <mettle/suite/compiled_suite.hpp>
#include <mettle/driver/log/core.hpp>
#include <mettle/driver/detail/export.hpp>

// Ignore warnings from MSVC about DLL interfaces.
#if defined(_MSC_VER) && !defined(__clang__)
#  pragma warning(push)
#  pragma warning(disable:4251)
#endif

namespace mettle {

  class METTLE_PUBLIC subprocess_test_runner {
  public:
    using timeout_t = std::optional<std::chrono::milliseconds>;

    subprocess_test_runner(timeout_t timeout = {}) : timeout_(timeout) {}

    template<class Rep, class Period>
    subprocess_test_runner(std::chrono::duration<Rep, Period> timeout)
      : timeout_(timeout) {}

    test_result
    operator ()(const test_info &test, log::test_output &output) const;
  private:
    timeout_t timeout_;
  };

#ifndef _WIN32

  using fd_type = int;
  int make_fd_private(int fd);

#else

  using fd_type = HANDLE;
  METTLE_PUBLIC int make_fd_private(HANDLE handle);

  METTLE_PUBLIC const test_info *
  find_test(const suites_list &suites, test_uid id);

  METTLE_PUBLIC int run_single_test(const test_info &test, HANDLE log_pipe);

#endif

} // namespace mettle

#if defined(_MSC_VER) && !defined(__clang__)
#  pragma warning(pop)
#endif

#endif
