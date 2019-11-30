#include "run_test_files.hpp"

#include "log_pipe.hpp"

#ifndef _WIN32
#  include "posix/run_test_file.hpp"
namespace platform = mettle::posix;
#else
#  include "windows/run_test_file.hpp"
namespace platform = mettle::windows;
#endif

namespace mettle {

  void run_test_files(
    const std::vector<test_file> &files, log::file_logger &logger,
    const std::vector<std::string> &args
  ) {
    using namespace platform;
    logger.started_run();

    for(const auto &file : files) {
      logger.started_file(file);

      std::vector<std::string> final_args = file.args();
      final_args.insert(final_args.end(), args.begin(), args.end());
      auto result = run_test_file(std::move(final_args), log::pipe(logger));

      if(result.passed)
        logger.ended_file(file);
      else
        logger.failed_file(file, result.message);
    }

    logger.ended_run();
  }

} // namespace mettle
