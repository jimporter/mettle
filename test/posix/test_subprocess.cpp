#include <mettle.hpp>
using namespace mettle;

#include <thread>

#include <fcntl.h>

#include <mettle/driver/exit_code.hpp>
#include <mettle/driver/posix/scoped_pipe.hpp>
#include <mettle/driver/posix/scoped_signal.hpp>
#include <mettle/driver/posix/subprocess.hpp>
using namespace mettle::posix;

#include "errno.hpp"

struct read_into_fixture {
  scoped_pipe pipe[2];
  std::string results[2];
  std::vector<readfd> readfds;
};

void sighandler(int) {}

suite<> test_subprocess("posix subprocess utilities", [](auto &_) {
  subsuite<>(_, "make_timeout_monitor()", [](auto &_) {
    _.test("test process exits first", []() {
      expect([]() {
        make_timeout_monitor(std::chrono::milliseconds(100));
        _exit(0);
      }, exited(0));
    });

    _.test("timer process exits first", []() {
      expect([]() {
        make_timeout_monitor(std::chrono::milliseconds(100));
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        _exit(0);
      }, exited(exit_code::timeout));
    });
  });

  subsuite<read_into_fixture>(_, "read_into()", [](auto &_) {
    _.setup([](read_into_fixture &f) {
      expect("open pipe 1", f.pipe[0].open(), equal_to(0));
      expect("open pipe 2", f.pipe[1].open(), equal_to(0));

      f.readfds = {
        {f.pipe[0].read_fd, &f.results[0]},
        {f.pipe[1].read_fd, &f.results[1]}
      };
    });

    _.test("read until fds close", [](read_into_fixture &f) {
      std::thread t([&f]() {
        write(f.pipe[0].write_fd, "pipe 1", 6);
        write(f.pipe[1].write_fd, "pipe 2", 6);
        f.pipe[0].close_write();
        f.pipe[1].close_write();
      });
      t.detach();

      expect(read_into(f.readfds, nullptr, nullptr), equal_to(0));

      expect(f.results[0], equal_to("pipe 1"));
      expect(f.results[1], equal_to("pipe 2"));
    });

    _.test("read until timeout", [](read_into_fixture &f) {
      std::thread t([&f]() {
        write(f.pipe[0].write_fd, "pipe 1", 6);
        std::this_thread::sleep_for(std::chrono::milliseconds(250));
        write(f.pipe[1].write_fd, "pipe 2", 6);
      });
      t.detach();

      timespec timeout = {0, 100*1000*1000 /* 10 ms */};
      expect(read_into(f.readfds, &timeout, nullptr), equal_to(0));

      expect(f.results[0], equal_to("pipe 1"));
      expect(f.results[1], equal_to(""));
    });

    attributes sigtest_attrs;
#ifdef __APPLE__
    sigtest_attrs.insert(skip("pselect(2) is buggy on OS X"));
#endif

    _.test("read until signal", sigtest_attrs, [](read_into_fixture &f) {
      scoped_sigprocmask mask;
      mask.push(SIG_BLOCK, SIGUSR1);

      int pid;
      if((pid = fork()) < 0)
        throw std::system_error(errno, std::system_category());
      if(pid == 0) {
        write(f.pipe[0].write_fd, "pipe 1", 6);
        write(f.pipe[1].write_fd, "pipe 2", 6);
        kill(getppid(), SIGUSR1);
        _exit(0);
      }

      scoped_sigaction sig;
      expect("set sigaction", sig.open(SIGUSR1, sighandler), equal_to(0));

      sigset_t empty;
      sigemptyset(&empty);
      expect(read_into(f.readfds, nullptr, &empty),
             all( equal_to(-1), equal_errno(EINTR) ));

      expect(f.results[0], equal_to("pipe 1"));
      expect(f.results[1], equal_to("pipe 2"));
    });
  });

  subsuite<scoped_pipe>(_, "send_pgid()/recv_pgid()", [](auto &_) {
    _.setup([](scoped_pipe &pipe) {
      expect("open pipe", pipe.open(), equal_to(0));
    });

    _.test("successful transfer", [](scoped_pipe &pipe) {
      expect("send pgid", send_pgid(pipe.write_fd, 123), equal_to(0));

      int pgid;
      expect("recv pgid", recv_pgid(pipe.read_fd, &pgid), equal_to(0));
      expect("check pgid", pgid, equal_to(123));
    });

    _.test("write end closed", [](scoped_pipe &pipe) {
      expect("close write", pipe.close_write(), equal_to(0));
      int pgid;
      expect("recv pgid", recv_pgid(pipe.read_fd, &pgid),
             all( equal_to(-1), equal_errno(EIO) ));
    });
  });
});
