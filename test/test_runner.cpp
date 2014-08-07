#include <mettle.hpp>
using namespace mettle;

#include <signal.h>

#include <chrono>
#include <iostream>
#include <thread>

#include <mettle/runner.hpp>
#include <mettle/log/core.hpp>
#include "../src/libmettle/forked_test_runner.hpp"

struct my_test_logger : log::test_logger {
  my_test_logger()
    : tests_run(0), tests_passed(0), tests_failed(0), tests_skipped(0) {}

  virtual void started_run() {}
  virtual void ended_run() {}

  virtual void started_suite(const std::vector<std::string> &) {}
  virtual void ended_suite(const std::vector<std::string> &) {}

  virtual void started_test(const log::test_name &) {
    tests_run++;
  }
  virtual void passed_test(const log::test_name &, const log::test_output &) {
    tests_passed++;
  }
  virtual void failed_test(const log::test_name &, const std::string &,
                           const log::test_output &) {
    tests_failed++;
  }
  virtual void skipped_test(const log::test_name &) {
    tests_skipped++;
  }

  size_t tests_run, tests_passed, tests_failed, tests_skipped;
};


struct forked_fixture {
  forked_fixture() : runner(std::chrono::seconds(1)) {}

  operator forked_test_runner &() {
    return runner;
  }

  forked_test_runner runner;
};

suite<forked_fixture> test_fork("forked_test_runner", [](auto &_) {

  subsuite<log::test_output>(_, "run one test", [](auto &_) {
    _.test("passing test", [](forked_test_runner &runner,
                              log::test_output &output) {
      auto s = make_suite<>("inner", [](auto &_){
        _.test("test", []() {});
      });

      for(const auto &t : s) {
        auto result = runner(t.function, output);
        expect(result.passed, equal_to(true));
        expect(result.message, equal_to(""));
      }
    });

    _.test("failing test", [](forked_test_runner &runner,
                              log::test_output &output) {
      auto s = make_suite<>("inner", [](auto &_){
        _.test("test", []() {
          expect(true, equal_to(false));
        });
      });

      for(const auto &t : s) {
        auto result = runner(t.function, output);
        expect(result.passed, equal_to(false));
      }
    });

    _.test("aborting test", [](forked_test_runner &runner,
                               log::test_output &output) {
      auto s = make_suite<>("inner", [](auto &_){
        _.test("test", []() {
          abort();
        });
      });

      for(const auto &t : s) {
        auto result = runner(t.function, output);
        expect(result.passed, equal_to(false));
        expect(result.message, equal_to(strsignal(SIGABRT)));
      }
    });

    _.test("segfaulting test", [](forked_test_runner &runner,
                                  log::test_output &output) {
      auto s = make_suite<>("inner", [](auto &_){
        _.test("test", []() {
          raise(SIGSEGV);
        });
      });

      for(const auto &t : s) {
        auto result = runner(t.function, output);
        expect(result.passed, equal_to(false));
        expect(result.message, equal_to(strsignal(SIGSEGV)));
      }
    });

    _.test("timed out test", [](forked_test_runner &runner,
                                log::test_output &output) {
      auto s = make_suite<>("inner", [](auto &_){
        _.test("test", []() {
          std::this_thread::sleep_for(std::chrono::seconds(2));
        });
      });

      for(const auto &t : s) {
        auto result = runner(t.function, output);
        expect(result.passed, equal_to(false));
        expect(result.message, equal_to("Timed out after 1000 ms"));
      }
    });

    _.test("test with stdout", [](forked_test_runner &runner,
                                  log::test_output &output) {
      auto s = make_suite<>("inner", [](auto &_){
        _.test("test", []() {
          std::cout << "stdout";
        });
      });

      for(const auto &t : s) {
        auto result = runner(t.function, output);
        expect(output.stdout, equal_to("stdout"));
      }
    });

    _.test("test with stdout/stderr", [](forked_test_runner &runner,
                                         log::test_output &output) {
      auto s = make_suite<>("inner", [](auto &_){
        _.test("test", []() {
          std::cout << "stdout";
          std::cerr << "stderr";
        });
      });

      for(const auto &t : s) {
        auto result = runner(t.function, output);
        expect(output.stdout, equal_to("stdout"));
        expect(output.stderr, equal_to("stderr"));
      }
    });
  });

  subsuite<my_test_logger>(_, "run_tests()", [](auto &_) {
      _.test("suite of passing tests", [](forked_test_runner &runner,
                                          my_test_logger &logger) {
      auto s = make_suites<>("inner", [](auto &_){
        _.test("test 1", []() {});
        _.test("test 2", []() {});
        _.test("test 3", []() {});
      });

      run_tests(s, logger, runner);
      expect(logger.tests_run, equal_to(3));
      expect(logger.tests_passed, equal_to(3));
      expect(logger.tests_skipped, equal_to(0));
      expect(logger.tests_failed, equal_to(0));
    });

    _.test("suite with failing tests", [](forked_test_runner &runner,
                                          my_test_logger &logger) {
      auto s = make_suites<>("inner", [](auto &_){
        _.test("test 1", []() {
          expect(true, equal_to(false));
        });
        _.test("test 2", []() {});
        _.test("test 3", []() {});
      });

      run_tests(s, logger, runner);

      expect(logger.tests_run, equal_to(3));
      expect(logger.tests_passed, equal_to(2));
      expect(logger.tests_skipped, equal_to(0));
      expect(logger.tests_failed, equal_to(1));
    });


    _.test("suite with skipped tests", [](forked_test_runner &runner,
                                          my_test_logger &logger) {
      auto s = make_suites<>("inner", [](auto &_){
        _.test("test 1", []() {});
        _.skip_test("test 2", []() {});
        _.test("test 3", []() {});
      });

      run_tests(s, logger, runner);

      expect(logger.tests_run, equal_to(3));
      expect(logger.tests_passed, equal_to(2));
      expect(logger.tests_skipped, equal_to(1));
      expect(logger.tests_failed, equal_to(0));
    });

    _.test("crashing tests don't crash framework", [](
             forked_test_runner &runner, my_test_logger &logger
    ) {
      auto s = make_suites<>("inner", [](auto &_){
        _.test("test 1", []() {});
        _.test("test 2", []() {
          abort();
        });
        _.test("test 3", []() {});
      });

      run_tests(s, logger, runner);

      expect(logger.tests_run, equal_to(3));
      expect(logger.tests_passed, equal_to(2));
      expect(logger.tests_skipped, equal_to(0));
      expect(logger.tests_failed, equal_to(1));
    });
  });
});
