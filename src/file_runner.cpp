#include "file_runner.hpp"

#include <sys/resource.h>
#include <sys/wait.h>

#include <chrono>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#include "log_pipe.hpp"
#include "scoped_pipe.hpp"

namespace mettle {

namespace detail {
  std::unique_ptr<char *[]>
  make_argv(const std::vector<std::string> &argv) {
    auto real_argv = std::make_unique<char *[]>(argv.size() + 1);
    for(size_t i = 0; i != argv.size(); i++)
      real_argv[i] = const_cast<char*>(argv[i].c_str());
    return real_argv;
  }

  [[noreturn]] void
  child_failed(int fd, const std::string &file) {
    // We know this is single-threaded, since we're the child of a fork(),
    // so strerror is safe.
    const char *err = strerror(errno);

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

  inline void parent_failed(log::pipe &logger, const std::string &file) {
    // `mettle` is single-threaded, since mixing fork() and threads is evil, so
    // strerror is safe.
    logger.failed_file(file, strerror(errno));
  }

  void run_test_file(
    const std::string &file, log::pipe &logger, const run_options &options
  ) {
    scoped_pipe message_pipe;
    message_pipe.open();

    rlimit lim;
    if(getrlimit(RLIMIT_NOFILE, &lim) < 0)
      return parent_failed(logger, file);
    int max_fd = lim.rlim_cur - 1;

    std::vector<std::string> args = {file, "--child", std::to_string(max_fd)};

    if(options.timeout) {
      args.push_back("--timeout");
      args.push_back(std::to_string(options.timeout->count()));
    }

    if(options.no_fork)
      args.push_back("--no-fork");

    auto argv = make_argv(args);

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

      execv(file.c_str(), argv.get());
      child_failed(max_fd, file);
    }
    else {
      if(message_pipe.close_write() < 0)
        return parent_failed(logger, file);

      std::exception_ptr except;
      try {
        namespace io = boost::iostreams;
        io::stream<io::file_descriptor_source> fds(
          message_pipe.read_fd, io::never_close_handle
        );
        while(!fds.eof())
          logger(fds);
      }
      catch(...) {
        except = std::current_exception();
      }

      int status;
      if(waitpid(pid, &status, 0) < 0)
        return parent_failed(logger, file);

      if(WIFEXITED(status)) {
        int exit_code = WEXITSTATUS(status);
        if(exit_code) {
          std::stringstream ss;
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
      }
      else if(WIFSIGNALED(status)) {
        logger.failed_file(file, strsignal(WTERMSIG(status)));
      }
      else { // WIFSTOPPED
        logger.failed_file(file, "Stopped");
      }

      return;
    }
  }
}

void run_test_files(
  const std::vector<std::string> &files, log::test_logger &logger,
  const run_options &options
) {
  logger.started_run();
  log::pipe pipe(logger);
  for(const auto &file : files)
    detail::run_test_file(file, pipe, options);
  logger.ended_run();
}

} // namespace mettle
