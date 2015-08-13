#ifndef INC_METTLE_SRC_POSIX_RUN_TEST_FILE_HPP
#define INC_METTLE_SRC_POSIX_RUN_TEST_FILE_HPP

#include <string>
#include <vector>

#include "../log_pipe.hpp"
#include "../run_test_files.hpp"

namespace mettle {

namespace posix {
  file_result run_test_file(std::vector<std::string> args, log::pipe &logger);

  inline file_result
  run_test_file(std::vector<std::string> args, log::pipe &&logger) {
    return run_test_file(std::move(args), logger);
  }
}

} // namespace mettle

#endif
