#ifndef INC_METTLE_FILE_RUNNER_HPP
#define INC_METTLE_FILE_RUNNER_HPP

#include <fcntl.h>
#include <sys/wait.h>

#include <iomanip>
#include <istream>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#include <bencode.hpp>

#include "scoped_pipe.hpp"
#include "log/core.hpp"
#include "log/pipe.hpp"

namespace mettle {

namespace detail {

  inline void run_test_file(const std::string &file, log::pipe &logger) {
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

      execl(file.c_str(), file.c_str(), "--child", nullptr);
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

inline void run_test_files(const std::vector<std::string> &files,
                           log::test_logger &logger) {
  logger.start_run();
  log::pipe pipe(logger);
  for(const auto &file : files)
    detail::run_test_file(file, pipe);
  logger.end_run();
}

} // namespace mettle

#endif
