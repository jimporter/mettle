#ifndef INC_METTLE_FILE_RUNNER_HPP
#define INC_METTLE_FILE_RUNNER_HPP

#include <fcntl.h>
#include <sys/wait.h>

#include <iomanip>
#include <istream>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#include "scoped_pipe.hpp"
#include "log/core.hpp"

namespace mettle {

namespace detail {

  // This code sucks. See <include/mettle/log/child.hpp> for more details.
  std::vector<std::string> read_suites(std::istream &s) {
    size_t size;
    s >> size;

    std::vector<std::string> suites(size);
    for(size_t i = 0; i < size; i++)
      s >> std::quoted(suites[i]);
    return suites;
  }

  log::test_name read_test_name(std::istream &s) {
    log::test_name test;
    test.suites = read_suites(s);
    s >> std::quoted(test.test) >> test.id;
    return test;
  }

  log::test_output & read_test_output(std::istream &s,
                                      log::test_output &output) {
    std::string stdout, stderr;
    s >> std::quoted(stdout) >> std::quoted(stderr);
    output.stdout << stdout;
    output.stderr << stderr;
    return output;
  }

  std::string read_test_message(std::istream &s) {
    std::string message;
    s >> std::quoted(message);
    return message;
  }

  void pipe_to_logger(log::test_logger &logger, std::istream &s) {
    std::string event;
    s >> event;

    if(event == "start_suite") {
      logger.start_suite(read_suites(s));
    }
    else if(event == "end_suite") {
      logger.end_suite(read_suites(s));
    }
    else if(event == "start_test") {
      logger.start_test(read_test_name(s));
    }
    else if(event == "passed_test") {
      log::test_output output;
      logger.passed_test(read_test_name(s), read_test_output(s, output));
    }
    else if(event == "failed_test") {
      log::test_output output;
      logger.failed_test(read_test_name(s), read_test_message(s),
                         read_test_output(s, output));
    }
    else if(event == "skipped_test") {
      logger.skipped_test(read_test_name(s));
    }
  }

  void run_test_file(const std::string &file, log::test_logger &logger) {
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
      exit(1);
    }
    else {
      if(stdout_pipe.close_write() < 0)
        goto parent_fail;

      namespace io = boost::iostreams;
      io::stream<io::file_descriptor_source> fds(
        stdout_pipe.read_fd, io::never_close_handle
      );

      while(!fds.eof())
        pipe_to_logger(logger, fds);

      if(waitpid(pid, nullptr, 0) < 0)
        goto parent_fail;
      return;
    }

  parent_fail:
    exit(1); // TODO: what should we do here?
  }
}

void run_test_files(const std::vector<std::string> &files,
                    log::test_logger &logger) {
  logger.start_run();
  for(const auto &file : files)
    detail::run_test_file(file, logger);
  logger.end_run();
}

} // namespace mettle

#endif
