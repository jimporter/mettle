#ifndef INC_METTLE_FILE_RUNNER_HPP
#define INC_METTLE_FILE_RUNNER_HPP

#include <chrono>

#include <mettle/optional.hpp>
#include <mettle/log/core.hpp>

namespace mettle {

void run_test_files(
  const std::vector<std::string> &files, log::test_logger &logger,
  METTLE_OPTIONAL_NS::optional<std::chrono::milliseconds> timeout =
  METTLE_OPTIONAL_NS::optional<std::chrono::milliseconds>()
);

template<class Rep, class Period>
inline void run_test_files(
  const std::vector<std::string> &files, log::test_logger &logger,
  std::chrono::duration<Rep, Period> timeout
) {
  METTLE_OPTIONAL_NS::optional<std::chrono::milliseconds> opt(timeout);
  run_test_files(files, logger, opt);
}

} // namespace mettle

#endif
