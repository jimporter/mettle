#include "run_test_files.hpp"

#include <glob.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include <stdexcept>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>
#include <boost/program_options.hpp>
#include <boost/tokenizer.hpp>

#include <mettle/driver/scoped_pipe.hpp>

#include "log_pipe.hpp"
#include "utils.hpp"

namespace mettle {

test_file::test_file(std::string command) : command_(std::move(command)) {
  using separator = boost::escaped_list_separator<char>;
  boost::tokenizer<separator> tok(
    command_.begin(), command_.end(), separator("\\", " \t", "'\"")
  );

  for(auto &&token : tok) {
    if(token.empty())
      continue;

    if(token.find_first_of("?*[") != std::string::npos) {
      glob_t g;
      if(glob(token.c_str(), 0, nullptr, &g) != 0)
        throw std::runtime_error("invalid glob \"" + token + "\"");
      for(size_t i = 0; i != g.gl_pathc; i++)
        args_.push_back(g.gl_pathv[i]);
      globfree(&g);
    }
    else {
      args_.push_back(token);
    }
  }
}

void validate(boost::any &v, const std::vector<std::string> &values,
              test_file*, int) {
  using namespace boost::program_options;
  validators::check_first_occurrence(v);
  const std::string &val = validators::get_single_string(values);

  try {
    v = test_file(val);
  }
  catch(...) {
    boost::throw_exception(invalid_option_value(val));
  }
}

namespace detail {
  std::unique_ptr<char *[]>
  make_argv(const std::vector<std::string> &argv) {
    auto real_argv = std::make_unique<char *[]>(argv.size() + 1);
    for(size_t i = 0; i != argv.size(); i++)
      real_argv[i] = const_cast<char*>(argv[i].c_str());
    return real_argv;
  }

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
    scoped_pipe message_pipe;
    message_pipe.open();

    rlimit lim;
    if(getrlimit(RLIMIT_NOFILE, &lim) < 0)
      return parent_failed(logger, file);
    int max_fd = lim.rlim_cur - 1;

    std::vector<std::string> final_args = file.args();
    final_args.insert(final_args.end(), args.begin(), args.end());
    final_args.insert(final_args.end(), {"--child", std::to_string(max_fd)});
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
      if(message_pipe.close_write() < 0)
        return parent_failed(logger, file);

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
      if(waitpid(pid, &status, 0) < 0)
        return parent_failed(logger, file);

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
      else if(WIFSIGNALED(status)) {
        logger.failed_file(file, strsignal(WTERMSIG(status)));
      }
      else { // WIFSTOPPED
        kill(pid, SIGKILL);
        logger.failed_file(file, strsignal(WSTOPSIG(status)));
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
    detail::run_test_file(file, pipe, args);
  logger.ended_run();
}

} // namespace mettle
