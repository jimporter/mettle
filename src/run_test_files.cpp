#include "run_test_files.hpp"

#include <cstdint>

#include <sys/resource.h>
#include <sys/wait.h>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#include <mettle/driver/posix/scoped_pipe.hpp>

#include "log_pipe.hpp"
#include "utils.hpp"

namespace mettle {

namespace {
  namespace {
    inline void parent_failed(log::pipe &logger, const std::string &file) {
      logger.failed_file(file, err_string(errno));
    }

    [[noreturn]] void
    child_failed(int fd, const std::string &file) {
      auto err = err_string(errno);

      try {
        namespace io = boost::iostreams;
        io::stream<io::file_descriptor_sink> stream(fd, io::never_close_handle);
        bencode::encode_dict(stream,
          "event", "failed_file",
          "file", file,
          "message", err
        );
        stream.flush();
        _exit(0);
      }
      catch(...) {
        _exit(128);
      }
    }
  }

  void run_test_file(const test_file &file, log::pipe &logger,
                     const std::vector<std::string> &args) {
    logger.started_file(file);
    posix::scoped_pipe message_pipe;
    message_pipe.open();

    rlimit lim;
    if(getrlimit(RLIMIT_NOFILE, &lim) < 0)
      return parent_failed(logger, file);
    int max_fd = lim.rlim_cur - 1;

    std::vector<std::string> final_args = file.args();
    final_args.insert(final_args.end(), args.begin(), args.end());
    final_args.insert(final_args.end(), {
      "--output-fd", std::to_string(max_fd)
    });
    auto argv = make_argv(final_args);

    pid_t pid;
    if((pid = fork()) < 0)
      return parent_failed(logger, file);

    if(pid == 0) {
      if(message_pipe.close_read() < 0)
        child_failed(message_pipe.write_fd, file);

      if(message_pipe.write_fd != max_fd) {
        if(dup2(message_pipe.write_fd, max_fd) < 0)
          child_failed(message_pipe.write_fd, file);

        if(message_pipe.close_write() < 0)
          child_failed(max_fd, file);
      }

      execvp(argv[0], argv.get());
      child_failed(max_fd, file);
    }
    else {
      if(message_pipe.close_write() < 0) {
        kill(pid, SIGKILL);
        return parent_failed(logger, file);
      }

      std::exception_ptr except;
      try {
        namespace io = boost::iostreams;
        io::stream<io::file_descriptor_source> fds(
          message_pipe.read_fd, io::never_close_handle
        );
        while(fds.peek() != EOF)
          logger(fds);
      }
      catch(...) {
        except = std::current_exception();
      }

      int status;
      if(waitpid(pid, &status, 0) < 0) {
        kill(pid, SIGKILL);
        return parent_failed(logger, file);
      }

      if(WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        if(exit_code) {
          std::ostringstream ss;
          ss << "Exited with status " << exit_code;
          logger.failed_file(file, ss.str());
        }
        else if(except) {
          try {
            std::rethrow_exception(except);
          }
          catch(const std::exception &e) {
            logger.failed_file(file, e.what());
          }
        }
        else {
          logger.ended_file(file);
        }
      }
      else { // WIFSIGNALED
        logger.failed_file(file, strsignal(WTERMSIG(status)));
      }
    }
  }
}

void run_test_files(
  const std::vector<test_file> &files, log::file_logger &logger,
  const std::vector<std::string> &args
) {
  logger.started_run();
  log::pipe pipe(logger);
  for(const auto &file : files)
    run_test_file(file, pipe, args);
  logger.ended_run();
}

} // namespace mettle
