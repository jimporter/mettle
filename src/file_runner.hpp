#ifndef INC_METTLE_FILE_RUNNER_HPP
#define INC_METTLE_FILE_RUNNER_HPP

#include <chrono>

#include <mettle/log/core.hpp>

#include "optional.hpp"

namespace mettle {

void run_test_files(
  const std::vector<std::string> &files, log::file_logger &logger,
  const std::vector<std::string> &args = std::vector<std::string>{}
);

} // namespace mettle

#endif
