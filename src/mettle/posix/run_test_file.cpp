#include "run_test_file.hpp"

#include <cstdint>

#include <signal.h>
#include <sys/resource.h>
#include <sys/wait.h>

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#include <mettle/driver/exit_code.hpp>
#include <mettle/driver/posix/scoped_pipe.hpp>

#include "../../err_string.hpp"

namespace mettle::posix {

  namespace {
    inline file_result parent_failed() {
      return {false, err_string(errno)};
    }

    [[noreturn]] void
    child_failed(int fd, const std::string &file) {
      auto err = err_string(errno);

      try {
        namespace io = boost::iostreams;
        io::stream<io::file_descriptor_sink> stream(
          fd, io::never_close_handle
        );
        bencode::encode(stream, bencode::dict_view{
          {"event", "failed_file"},
          {"file", file},
          {"message", err}
        });
        stream.flush();
        _exit(exit_code::success);
      } catch(...) {
        _exit(exit_code::fatal);
      }
    }

    std::unique_ptr<char *[]>
    make_argv(const std::vector<std::string> &argv) {
      auto real_argv = std::make_unique<char *[]>(argv.size() + 1);
      for(std::size_t i = 0; i != argv.size(); i++)
        real_argv[i] = const_cast<char*>(argv[i].c_str());
      return real_argv;
    }
  }

  file_result run_test_file(std::vector<std::string> args, log::pipe &logger) {
    posix::scoped_pipe message_pipe;
    if(message_pipe.open() < 0)
      return parent_failed();

    rlimit lim;
    if(getrlimit(RLIMIT_NOFILE, &lim) < 0)
      return parent_failed();
    int max_fd = lim.rlim_cur - 1;

    args.insert(args.end(), { "--output-fd", std::to_string(max_fd) });
    auto argv = make_argv(args);

    pid_t pid;
    if((pid = fork()) < 0)
      return parent_failed();

    if(pid == 0) {
      if(message_pipe.close_read() < 0)
        child_failed(message_pipe.write_fd, args[0]);

      if(message_pipe.write_fd != max_fd) {
        if(dup2(message_pipe.write_fd, max_fd) < 0)
          child_failed(message_pipe.write_fd, args[0]);

        if(message_pipe.close_write() < 0)
          child_failed(max_fd, args[0]);
      }

      execvp(argv[0], argv.get());
      child_failed(max_fd, args[0]);
    } else {
      if(message_pipe.close_write() < 0) {
        kill(pid, SIGKILL);
        return parent_failed();
      }

      std::exception_ptr except;
      try {
        namespace io = boost::iostreams;
        io::stream<io::file_descriptor_source> fds(
          message_pipe.read_fd, io::never_close_handle
        );
        while(fds.peek() != EOF)
          logger(fds);
      } catch(...) {
        except = std::current_exception();
      }

      int status;
      if(waitpid(pid, &status, 0) < 0) {
        kill(pid, SIGKILL);
        return parent_failed();
      }

      if(WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        if(exit_status != exit_code::success) {
          std::ostringstream ss;
          ss << "Exited with status " << exit_status;
          return {false, ss.str()};
        } else if(except) {
          try {
            std::rethrow_exception(except);
          } catch(const std::exception &e) {
            return {false, e.what()};
          }
        } else {
          return {true, ""};
        }
      } else { // WIFSIGNALED
        return {false, strsignal(WTERMSIG(status))};
      }
    }
  }

} // namespace mettle::posix
