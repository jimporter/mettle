#include <mettle.hpp>
using namespace mettle;

#include <signal.h>

#include <chrono>
#include <iostream>
#include <thread>

#include <mettle/driver/run_tests.hpp>
#include <mettle/driver/log/core.hpp>
#include "../src/libmettle/forked_test_runner.hpp"

struct test_event_logger : log::test_logger {
  test_event_logger() {}

  void started_run() {
    events.push_back("started_run");
  }
  void ended_run() {
    events.push_back("ended_run");
  }

  void started_suite(const std::vector<std::string> &) {
    events.push_back("started_suite");
  }
  void ended_suite(const std::vector<std::string> &) {
    events.push_back("ended_suite");
  }

  void started_test(const test_name &) {
    events.push_back("started_test");
  }
  void passed_test(const test_name &, const log::test_output &,
                   log::test_duration) {
    events.push_back("passed_test");
  }
  void failed_test(const test_name &, const std::string &,
                   const log::test_output &, log::test_duration) {
    events.push_back("failed_test");
  }
  void skipped_test(const test_name &, const std::string &) {
    events.push_back("skipped_test");
  }

  std::vector<std::string> events;
};

struct ftr_factory {
  template<typename T>
  T make() {
    return T(std::chrono::milliseconds(500));
  }
};

suite<forked_test_runner> test_fork("forked_test_runner", ftr_factory{},
                                    [](auto &_) {

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
        auto then = std::chrono::steady_clock::now();
        auto result = runner(t.function, output);
        auto now = std::chrono::steady_clock::now();

        expect(result.passed, equal_to(false));
        expect(result.message, equal_to("Timed out after 500 ms"));
        expect(now - then, less(std::chrono::seconds(1)));
      }
    });

    _.test("test with timed out child", [](forked_test_runner &runner,
                                           log::test_output &output) {
      auto s = make_suite<>("inner", [](auto &_){
        _.test("test", []() {
          pid_t pid;
          if((pid = fork()) < 0)
            throw std::system_error(errno, std::system_category());
          if(pid == 0) {
            std::this_thread::sleep_for(std::chrono::seconds(2));
          }
        });
      });

      for(const auto &t : s) {
        auto then = std::chrono::steady_clock::now();
        auto result = runner(t.function, output);
        auto now = std::chrono::steady_clock::now();

        expect(result.passed, equal_to(true));
        expect(now - then, less(std::chrono::seconds(1)));
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

  subsuite<test_event_logger>(_, "run_tests()", [](auto &_) {

    _.test("single suite", [](forked_test_runner &runner,
                              test_event_logger &logger) {
      auto s = make_suites<>("inner", [](auto &_){
        _.test("test 1", []() {});
        _.test("test 2", []() { expect(true, equal_to(false)); });
        _.test("test 3", {skip}, []() {});
      });

      std::vector<std::string> expected = {
        "started_run",
        "started_suite",
          "started_test",
          "passed_test",
          "started_test",
          "failed_test",
          "started_test",
          "skipped_test",
        "ended_suite",
        "ended_run"
      };

      run_tests(s, logger, runner);
      expect(logger.events, equal_to(expected));
    });

    _.test("multiple suites", [](forked_test_runner &runner,
                                 test_event_logger &logger) {
      auto s = make_suites<int, float>("inner", [](auto &_){
        _.test("test 1", [](const auto &) {});
      });

      std::vector<std::string> expected = {
        "started_run",
        "started_suite",
          "started_test",
          "passed_test",
        "ended_suite",
        "started_suite",
          "started_test",
          "passed_test",
        "ended_suite",
        "ended_run"
      };

      run_tests(s, logger, runner);
      expect(logger.events, equal_to(expected));
    });

    _.test("suite with subsuites", [](forked_test_runner &runner,
                                      test_event_logger &logger) {
      auto s = make_suites<>("inner", [](auto &_){
        _.test("test 1", []() {});
        subsuite<>(_, "subsuite 1", [](auto &_) {
          _.test("sub-test 1", []() {});
          subsuite<>(_, "sub-subsuite", [](auto &_) {
            _.test("sub-sub-test 1", []() {});
          });
        });
        subsuite<>(_, "subsuite 2", [](auto &_) {
          _.test("sub-test 2", []() {});
        });
      });

      std::vector<std::string> expected = {
        "started_run",
        "started_suite",
          "started_test",
          "passed_test",
          "started_suite",
            "started_test",
            "passed_test",
            "started_suite",
              "started_test",
              "passed_test",
            "ended_suite",
          "ended_suite",
          "started_suite",
            "started_test",
            "passed_test",
          "ended_suite",
        "ended_suite",
        "ended_run"
      };

      run_tests(s, logger, runner);
      expect(logger.events, equal_to(expected));
    });

    _.test("suite with hidden subsuites", [](forked_test_runner &runner,
                                             test_event_logger &logger) {
      bool_attr hide("hide");
      auto s = make_suites<>("inner", [&hide](auto &_){
        _.test("test 1", []() {});
        _.test("test 2", {hide}, []() {});
        subsuite<>(_, "subsuite 1", {hide}, [](auto &_) {
          _.test("sub-test 1", []() {});
          subsuite<>(_, "sub-subsuite", [](auto &_) {
            _.test("sub-sub-test 1", []() {});
          });
        });
        subsuite<>(_, "subsuite 2", [](auto &_) {
          _.test("sub-test 2", []() {});
        });
        subsuite<>(_, "subsuite 3", [&hide](auto &_) {
          _.test("sub-test 3", {hide}, []() {});
        });
      });

      std::vector<std::string> expected = {
        "started_run",
        "started_suite",
          "started_test",
          "passed_test",
          "started_suite",
            "started_test",
            "passed_test",
          "ended_suite",
        "ended_suite",
        "ended_run"
      };

      auto filter = [](
        const test_name &, const attributes &attrs
      ) -> filter_result {
        return attrs.find("hide") != attrs.end() ? test_action::hide :
          test_action::run;
      };
      run_tests(s, logger, runner, filter);
      expect(logger.events, equal_to(expected));
    });

    _.test("crashing tests don't crash framework", [](
      forked_test_runner &runner, test_event_logger &logger
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
