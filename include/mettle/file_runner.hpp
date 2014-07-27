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

namespace mettle {

namespace detail {

  // XXX: Put this stuff in a class somewhere?
  inline std::vector<std::string>
  read_suites(BENCODE_ANY_NS::any &suites) {
    using namespace BENCODE_ANY_NS;
    std::vector<std::string> result;
    for(auto &&i : any_cast<bencode::list &>(suites))
      result.push_back(std::move( any_cast<bencode::string &>(i) ));
    return result;
  }

  inline log::test_name read_test_name(BENCODE_ANY_NS::any &test) {
    using namespace BENCODE_ANY_NS;
    auto &data = any_cast<bencode::dict &>(test);
    return log::test_name{
      read_suites(data.at("suites")),
      std::move(any_cast<bencode::string &>( data.at("test")  )),
      static_cast<size_t>(any_cast<bencode::integer>( data.at("id") )),
    };
  }

  inline log::test_output read_test_output(BENCODE_ANY_NS::any &output) {
    using namespace BENCODE_ANY_NS;
    auto &data = any_cast<bencode::dict &>(output);
    return log::test_output{
      std::move(any_cast<bencode::string &>( data.at("stdout") )),
      std::move(any_cast<bencode::string &>( data.at("stderr") ))
    };
  }

  inline std::string read_test_message(BENCODE_ANY_NS::any &message) {
    using namespace BENCODE_ANY_NS;
    return std::move(any_cast<bencode::string &>(message));
  }

  inline void pipe_to_logger(log::test_logger &logger, std::istream &s) {
    using namespace BENCODE_ANY_NS;

    auto tmp = bencode::decode(s);
    if(tmp.empty())
      return;

    auto &data = any_cast<bencode::dict &>(tmp);
    auto &event = any_cast<bencode::string &>(data.at("event"));

    if(event == "start_suite") {
      logger.start_suite(read_suites(data.at("suites")));
    }
    else if(event == "end_suite") {
      logger.end_suite(read_suites(data.at("suites")));
    }
    else if(event == "start_test") {
      logger.start_test(read_test_name(data.at("test")));
    }
    else if(event == "passed_test") {
      logger.passed_test(read_test_name(data.at("test")),
                         read_test_output(data.at("output")));
    }
    else if(event == "failed_test") {
      logger.failed_test(read_test_name(data.at("test")),
                         read_test_message(data.at("message")),
                         read_test_output(data.at("output")));
    }
    else if(event == "skipped_test") {
      logger.skipped_test(read_test_name(data.at("test")));
    }
  }

  inline void run_test_file(const std::string &file, log::test_logger &logger) {
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

inline void run_test_files(const std::vector<std::string> &files,
                    log::test_logger &logger) {
  logger.start_run();
  for(const auto &file : files)
    detail::run_test_file(file, logger);
  logger.end_run();
}

} // namespace mettle

#endif
