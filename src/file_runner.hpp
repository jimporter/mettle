#ifndef INC_METTLE_FILE_RUNNER_HPP
#define INC_METTLE_FILE_RUNNER_HPP

#include <chrono>

#include <mettle/log/core.hpp>

#include "optional.hpp"

namespace mettle {

struct run_options {
  METTLE_OPTIONAL_NS::optional<std::chrono::milliseconds> timeout;
  bool no_fork = false;
};

void run_test_files(
  const std::vector<std::string> &files, log::test_logger &logger,
  const run_options &options = run_options{}
);

} // namespace mettle

#endif
