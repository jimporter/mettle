#include <mettle.hpp>
using namespace mettle;

#include <chrono>
#include <iostream>
#include <thread>

#include <signal.h>

#include <mettle/driver/run_tests.hpp>
#include <mettle/driver/posix/scoped_pipe.hpp>
#include "../../src/libmettle/posix/subprocess_test_runner.hpp"
#include "../test_event_logger.hpp"
using namespace mettle::posix;

struct sptr_factory {
  template<typename T>
  T make() {
    return T(std::chrono::milliseconds(250));
  }
};

suite<subprocess_test_runner>
test_fork("subprocess_test_runner", sptr_factory{}, [](auto &_) {

  subsuite<log::test_output>(_, "run one test", [](auto &_) {

    _.test("passing test", [](subprocess_test_runner &runner,
                              log::test_output &output) {
      auto s = make_suite<>("inner", [](auto &_){
        _.test("test", []() {});
      });

      auto result = runner(s.tests()[0], output);
      //expect(result.passed, equal_to(true));
      expect(result.message, equal_to(""));
    });

    _.test("failing test", [](subprocess_test_runner &runner,
                              log::test_output &output) {
      auto s = make_suite<>("inner", [](auto &_){
        _.test("test", []() {
          expect(true, equal_to(false));
        });
      });

      auto result = runner(s.tests()[0], output);
      expect(result.passed, equal_to(false));
    });

    _.test("aborting test", [](subprocess_test_runner &runner,
                               log::test_output &output) {
      auto s = make_suite<>("inner", [](auto &_){
        _.test("test", []() {
          abort();
        });
      });

      auto result = runner(s.tests()[0], output);
      expect(result.passed, equal_to(false));
      expect(result.message, equal_to(strsignal(SIGABRT)));
    });

    _.test("segfaulting test", [](subprocess_test_runner &runner,
                                  log::test_output &output) {
      auto s = make_suite<>("inner", [](auto &_){
        _.test("test", []() {
          raise(SIGSEGV);
        });
      });

      auto result = runner(s.tests()[0], output);
      expect(result.passed, equal_to(false));
      expect(result.message, equal_to(strsignal(SIGSEGV)));
    });

    _.test("timed out test", [](subprocess_test_runner &runner,
                                log::test_output &output) {
      auto s = make_suite<>("inner", [](auto &_){
        _.test("test", []() {
          std::this_thread::sleep_for(std::chrono::seconds(2));
        });
      });

      auto then = std::chrono::steady_clock::now();
      auto result = runner(s.tests()[0], output);
      auto now = std::chrono::steady_clock::now();

      expect(result.passed, equal_to(false));
      expect(result.message, equal_to("Timed out after 250 ms"));
      expect(now - then, less(std::chrono::seconds(1)));
    });

    _.test("test with timed out child", [](subprocess_test_runner &runner,
                                           log::test_output &output) {
      scoped_pipe block;
      block.open();

      auto s = make_suite<>("inner", [&block](auto &_){
        _.test("test", [&block]() {
          int pid;
          if((pid = fork()) < 0)
            throw std::system_error(errno, std::system_category());
          if(pid == 0) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
          }
        });
      });

      auto then = std::chrono::steady_clock::now();
      auto result = runner(s.tests()[0], output);
      // Try to read fromm our pipe; if the test's child process isn't killed,
      // this will block.
      block.close_write();
      char buf[1];
      read(block.read_fd, buf, 1);
      auto now = std::chrono::steady_clock::now();

      expect(result.passed, equal_to(true));
      expect(now - then, less(std::chrono::seconds(1)));
    });

    _.test("test with timed out child in new process group",
           [](subprocess_test_runner &runner, log::test_output &output) {
      auto s = make_suite<>("inner", [](auto &_){
        _.test("test", []() {
          pid_t pid;
          if((pid = fork()) < 0)
            throw std::system_error(errno, std::system_category());
          if(pid == 0) {
            // This will prevent the child process from being killed when the
            // parent is killed.
            setpgid(0, 0);
            std::this_thread::sleep_for(std::chrono::seconds(2));
          }
        });
      });

      auto then = std::chrono::steady_clock::now();
      auto result = runner(s.tests()[0], output);
      auto now = std::chrono::steady_clock::now();

      expect(result.passed, equal_to(true));
      expect(now - then, less(std::chrono::seconds(1)));
    });

    _.test("timed out test with timed out child",
           [](subprocess_test_runner &runner, log::test_output &output) {
      auto s = make_suite<>("inner", [](auto &_){
        _.test("test", []() {
          pid_t pid;
          if((pid = fork()) < 0)
            throw std::system_error(errno, std::system_category());

          std::this_thread::sleep_for(std::chrono::seconds(2));
        });
      });

      auto then = std::chrono::steady_clock::now();
      auto result = runner(s.tests()[0], output);
      auto now = std::chrono::steady_clock::now();

      expect(result.passed, equal_to(false));
      expect(result.message, equal_to("Timed out after 250 ms"));
      expect(now - then, less(std::chrono::seconds(1)));
    });

    _.test("test with stdout", [](subprocess_test_runner &runner,
                                  log::test_output &output) {
      auto s = make_suite<>("inner", [](auto &_){
        _.test("test", []() {
          std::cout << "stdout";
        });
      });

      auto result = runner(s.tests()[0], output);
      expect(output.stdout_log, equal_to("stdout"));
    });

    _.test("test with stdout/stderr", [](subprocess_test_runner &runner,
                                         log::test_output &output) {
      auto s = make_suite<>("inner", [](auto &_){
        _.test("test", []() {
          std::cout << "stdout";
          std::cerr << "stderr";
        });
      });

      auto result = runner(s.tests()[0], output);
      expect(output.stdout_log, equal_to("stdout"));
      expect(output.stderr_log, equal_to("stderr"));
    });

  });

  subsuite<test_event_logger>(_, "run_tests()", [](auto &_) {

    _.test("single suite", [](
      subprocess_test_runner &runner, test_event_logger &logger
    ) {
      auto s = make_suites<>("inner", [](auto &_){
        _.test("test", []() {});
      });

      std::vector<std::string> expected = {
        "started_run",
        "started_suite",
          "started_test",
          "passed_test",
        "ended_suite",
        "ended_run"
      };

      run_tests(s, logger, runner);
      expect(logger.events, equal_to(expected));
    });

    _.test("crashing tests don't crash framework", [](
      subprocess_test_runner &runner, test_event_logger &logger
    ) {
      auto s = make_suites<>("inner", [](auto &_){
        _.test("test 1", []() {});
        _.test("test 2", []() {
          abort();
        });
        _.test("test 3", []() {});
      });

      std::vector<std::string> expected = {
        "started_run",
        "started_suite",
          "started_test",
          "passed_test",
          "started_test",
          "failed_test",
          "started_test",
          "passed_test",
        "ended_suite",
        "ended_run"
      };

      run_tests(s, logger, runner);
      expect(logger.events, equal_to(expected));
    });

  });
});
