#ifndef INC_METTLE_FILE_RUNNER_HPP
#define INC_METTLE_FILE_RUNNER_HPP

#include <fcntl.h>
#include <sys/wait.h>

#include <iomanip>
#include <istream>
#include <chrono>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#include <bencode.hpp>

#include "optional.hpp"
#include "scoped_pipe.hpp"
#include "log/core.hpp"
#include "log/pipe.hpp"

namespace mettle {

namespace detail {

  inline void run_test_file(
    const std::string &file, log::pipe &logger,
    METTLE_OPTIONAL_NS::optional<std::chrono::milliseconds> timeout
  ) {
    scoped_pipe stdout_pipe;
    stdout_pipe.open();

    pid_t pid;
    if((pid = fork()) < 0)
      goto parent_fail;

    if(pid == 0) {
      if(stdout_pipe.close_read() < 0)
        goto child_fail;

      if(dup2(stdout_pipe.write_fd, STDOUT_FILENO) < 0)
        goto child_fail;

      if(timeout) {
        execl(file.c_str(), file.c_str(), "--child", "--timeout",
              std::to_string(timeout->count()).c_str(), nullptr);
      }
      else {
        execl(file.c_str(), file.c_str(), "--child", nullptr);
      }
    child_fail:
      exit(128);
    }
    else {
      if(stdout_pipe.close_write() < 0)
        goto parent_fail;

      namespace io = boost::iostreams;
      io::stream<io::file_descriptor_source> fds(
        stdout_pipe.read_fd, io::never_close_handle
      );

      while(!fds.eof())
        logger(fds);

      if(waitpid(pid, nullptr, 0) < 0)
        goto parent_fail;
      return;
    }

  parent_fail:
    exit(128); // TODO: what should we do here?
  }
}

inline void run_test_files(
  const std::vector<std::string> &files, log::test_logger &logger,
  METTLE_OPTIONAL_NS::optional<std::chrono::milliseconds> timeout =
  METTLE_OPTIONAL_NS::optional<std::chrono::milliseconds>()
) {
  logger.start_run();
  log::pipe pipe(logger);
  for(const auto &file : files)
    detail::run_test_file(file, pipe, timeout);
  logger.end_run();
}

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
