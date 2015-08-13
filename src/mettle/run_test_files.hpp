#ifndef INC_METTLE_SRC_RUN_TEST_FILES_HPP
#define INC_METTLE_SRC_RUN_TEST_FILES_HPP

#include <string>
#include <vector>

#include <mettle/driver/log/core.hpp>

#include "test_file.hpp"

namespace mettle {

struct file_result {
  bool passed;
  std::string message;
};

void run_test_files(
  const std::vector<test_file> &files, log::file_logger &logger,
  const std::vector<std::string> &args = {}
);

} // namespace mettle

#endif
