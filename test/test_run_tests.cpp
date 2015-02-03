#include <mettle.hpp>
using namespace mettle;

#include "test_event_logger.hpp"
#include <mettle/driver/run_tests.hpp>

suite<test_event_logger> test_run_tests("run_tests", [](auto &_) {

  _.test("single suite", [](test_event_logger &logger) {
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

    run_tests(s, logger, inline_test_runner);
    expect(logger.events, equal_to(expected));
  });

  _.test("multiple suites", [](test_event_logger &logger) {
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

    run_tests(s, logger, inline_test_runner);
    expect(logger.events, equal_to(expected));
  });

  _.test("suite with subsuites", [](test_event_logger &logger) {
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

    run_tests(s, logger, inline_test_runner);
    expect(logger.events, equal_to(expected));
  });

  _.test("suite with hidden subsuites", [](test_event_logger &logger) {
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
    run_tests(s, logger, inline_test_runner, filter);
    expect(logger.events, equal_to(expected));
  });

});
