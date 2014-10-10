#ifndef INC_METTLE_SRC_RUN_TEST_FILES_HPP
#define INC_METTLE_SRC_RUN_TEST_FILES_HPP

#include <chrono>

#include <mettle/driver/log/core.hpp>

namespace mettle {

void run_test_files(
  const std::vector<std::string> &files, log::file_logger &logger,
  const std::vector<std::string> &args = std::vector<std::string>{}
);

} // namespace mettle

#endif
