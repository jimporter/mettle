#ifndef INC_METTLE_SRC_POSIX_RUN_TEST_FILES_HPP
#define INC_METTLE_SRC_POSIX_RUN_TEST_FILES_HPP

#include <string>
#include <vector>

#include <mettle/driver/log/core.hpp>

#include "../test_file.hpp"

namespace mettle {

namespace posix {
  void run_test_files(
    const std::vector<test_file> &files, log::file_logger &logger,
    const std::vector<std::string> &args = std::vector<std::string>{}
  );
}

} // namespace mettle

#endif
