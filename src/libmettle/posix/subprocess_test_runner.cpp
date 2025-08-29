#include <mettle/driver/subprocess_test_runner.hpp>

#include <fcntl.h>
#include <poll.h>
#include <signal.h>
#include <string.h>
#include <sys/wait.h>

#include <sstream>

#include <bencode.hpp>

// Ignore warnings about deprecated implicit copy constructor.
#if defined(__clang__)
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wdeprecated"
#endif

#include <boost/iostreams/device/file_descriptor.hpp>
#include <boost/iostreams/stream.hpp>

#if defined(__clang__)
#  pragma clang diagnostic pop
#endif

#include <mettle/detail/source_location.hpp>
#include <mettle/driver/exit_code.hpp>
#include <mettle/driver/posix/scoped_pipe.hpp>
#include <mettle/driver/posix/scoped_signal.hpp>
#include <mettle/driver/posix/subprocess.hpp>

#include "../../err_string.hpp"

#ifdef METTLE_SAFE_EXIT
#  define EXIT_FUNC _exit
#else
#  define EXIT_FUNC exit
#endif

#ifdef METTLE_NO_SOURCE_LOCATION
#  define PARENT_FAILED() parent_failed(                                      \
     ::mettle::detail::source_location::current(__FILE__, __func__, __LINE__) \
   )
#else
#  define PARENT_FAILED() parent_failed()
#endif

namespace mettle {

  using namespace posix;

  namespace {
    pid_t test_pgid = 0;
    struct sigaction old_sigint, old_sigquit;

    void sig_handler(int signum) {
      assert(test_pgid != 0);
      killpg(test_pgid, signum);

      // Restore the previous signal action and re-raise the signal.
      struct sigaction *old_act = signum == SIGINT ? &old_sigint : &old_sigquit;
      sigaction(signum, old_act, nullptr);
      raise(signum);
    }

    void sig_chld(int) {}

    test_result parent_failed(detail::source_location loc =
                                detail::source_location::current()) {
      if(test_pgid)
        killpg(test_pgid, SIGKILL);
      test_pgid = 0;

      std::ostringstream ss;
      ss << "Fatal error at " << loc.file_name() << ":" << loc.line() << "\n"
         << err_string(errno);
      return {{ss.str()}};
    }

    [[noreturn]] inline void child_failed() {
      _exit(exit_code::fatal);
    }

    int fd_to_close;
    void atfork_close_fd() {
      close(fd_to_close);
    }
  }

  test_result subprocess_test_runner::operator ()(
    const test_info &test, log::test_output &output
  ) const {
    assert(test_pgid == 0);

    scoped_pipe stdout_pipe, stderr_pipe, pgid_pipe, log_pipe;
    if(stdout_pipe.open() < 0 ||
       stderr_pipe.open() < 0 ||
       pgid_pipe.open() < 0 ||
       log_pipe.open(O_CLOEXEC) < 0)
      return PARENT_FAILED();

    fflush(nullptr);

    scoped_sigprocmask mask;
    if(mask.push(SIG_BLOCK, SIGCHLD) < 0 ||
       mask.push(SIG_BLOCK, {SIGINT, SIGQUIT}) < 0)
      return PARENT_FAILED();

    pid_t pid;
    if((pid = fork()) < 0)
      return PARENT_FAILED();

    if(pid == 0) {
      if(mask.clear() < 0)
        child_failed();

      if(stdout_pipe.close_read() < 0 ||
         stderr_pipe.close_read() < 0 ||
         pgid_pipe.close_read() < 0 ||
         log_pipe.close_read() < 0)
        child_failed();

      if(stdout_pipe.move_write(STDOUT_FILENO) < 0 ||
         stderr_pipe.move_write(STDERR_FILENO) < 0)
        child_failed();

      // Make a new process group so we can kill the test and all its children
      // as a group.
      if(setpgid(0, 0) < 0)
        child_failed();

      if(send_pgid(pgid_pipe.write_fd, getpgid(0)) < 0)
        child_failed();

      if(pgid_pipe.close_write() < 0)
        child_failed();

      if(timeout_)
        make_timeout_monitor(*timeout_);

      auto failed = test.function();
      if(failed) {
        try {
          namespace io = boost::iostreams;
          io::stream<io::file_descriptor_sink> stream(
            log_pipe.write_fd, io::never_close_handle
          );
          stream.exceptions(stream.failbit | stream.badbit);
          bencode::encode(stream, failed->to_bencode<bencode::data_view>());
          stream.flush();
        } catch(...) {
          child_failed();
        }
      }

      fflush(nullptr);

      EXIT_FUNC(failed ? exit_code::failure : exit_code::success);
    } else {
      scoped_sigaction sigint, sigquit, sigchld;

      if(stdout_pipe.close_write() < 0 ||
         stderr_pipe.close_write() < 0 ||
         pgid_pipe.close_write() < 0 ||
         log_pipe.close_write() < 0)
        return PARENT_FAILED();

      if(recv_pgid(pgid_pipe.read_fd, &test_pgid) < 0)
        return PARENT_FAILED();

      if(sigaction(SIGINT, nullptr, &old_sigint) < 0 ||
         sigaction(SIGQUIT, nullptr, &old_sigquit) < 0)
        return PARENT_FAILED();

      if(sigint.open(SIGINT, sig_handler) < 0 ||
         sigquit.open(SIGQUIT, sig_handler) < 0 ||
         sigchld.open(SIGCHLD, sig_chld) < 0)
        return PARENT_FAILED();

      if(mask.pop() < 0)
        return PARENT_FAILED();

      std::string message;
      std::vector<readfd> dests = {
        {stdout_pipe.read_fd, &output.stdout_log},
        {stderr_pipe.read_fd, &output.stderr_log},
        {log_pipe.read_fd,    &message}
      };

      // Read from the piped stdout, stderr, and log. If we're interrupted
      // (probably by SIGCHLD), do one last non-blocking read to get any data we
      // might have missed.
      sigset_t empty;
      sigemptyset(&empty);
      if(read_into(dests, nullptr, &empty) < 0) {
        if(errno != EINTR)
          return PARENT_FAILED();
        timespec timeout = {0, 0};
        if(read_into(dests, &timeout, nullptr) < 0)
          return PARENT_FAILED();
      }

      int status;
      if(waitpid(pid, &status, 0) < 0)
        return PARENT_FAILED();

      // Make sure everything in the test's process group is dead. Don't worry
      // about reaping.
      killpg(test_pgid, SIGKILL);
      test_pgid = 0;

      if(WIFEXITED(status)) {
        int exit_status = WEXITSTATUS(status);
        if(exit_status == exit_code::timeout) {
          std::ostringstream ss;
          ss << "Timed out after " << timeout_->count() << " ms";
          return {{ss.str()}};
        } else if(exit_status == exit_code::success) {
          return std::nullopt;
        } else {
          return {test_failure::from_bencode(bencode::decode(message))};
        }
      } else { // WIFSIGNALED
        return {{ strsignal(WTERMSIG(status)) }};
      }
    }
  }

  int make_fd_private(int fd) {
    fd_to_close = fd;
    return pthread_atfork(nullptr, nullptr, atfork_close_fd);
  }

} // namespace mettle
