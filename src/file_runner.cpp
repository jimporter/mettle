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
  int execvec(const std::string &path, const std::vector<std::string> &argv) {
    auto real_argv = std::make_unique<char *[]>(argv.size() + 1);
    for(size_t i = 0; i != argv.size(); i++)
      real_argv[i] = const_cast<char*>(argv[i].c_str());
    return execv(path.c_str(), real_argv.get());
  }

  void run_test_file(
    const std::string &file, log::pipe &logger, const run_options &options
  ) {
    scoped_pipe stdout_pipe;
    stdout_pipe.open();

    pid_t pid;
    if((pid = fork()) < 0)
      goto parent_fail;

    if(pid == 0) {
      if(stdout_pipe.close_read() < 0)
        goto child_fail;

      rlimit lim;
      int fd;
      if(getrlimit(RLIMIT_NOFILE, &lim) < 0)
        goto child_fail;
      fd = lim.rlim_cur - 1;

      if(dup2(stdout_pipe.write_fd, fd) < 0)
        goto child_fail;

      {
        std::vector<std::string> argv = {file, "--child", std::to_string(fd)};

        if(options.timeout) {
          argv.push_back("--timeout");
          argv.push_back(std::to_string(options.timeout->count()));
        }

        if(options.no_fork)
          argv.push_back("--no-fork");

        execvec(file, argv);
      }
    child_fail:
      _exit(128);
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

      int status;
      if(waitpid(pid, &status, 0) < 0)
        goto parent_fail;

      if(WIFSIGNALED(status)) {
        std::cerr << strsignal(WTERMSIG(status)) << std::endl;
        goto parent_fail;
      }
      return;
    }

  parent_fail:
    exit(128); // TODO: what should we do here?
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
