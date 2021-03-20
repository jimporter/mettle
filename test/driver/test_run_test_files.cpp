#include <mettle.hpp>
using namespace mettle;

#include <cstdlib>

#include "../../src/mettle/log_pipe.hpp"
#include "../../src/mettle/run_test_files.hpp"

#ifndef _WIN32
#  include "../../src/mettle/posix/run_test_file.hpp"
namespace platform = mettle::posix;
std::string pathsep = "/";
#else
#  include "../../src/mettle/windows/run_test_file.hpp"
namespace platform = mettle::windows;
std::string pathsep = "\\";
#endif

#include "../test_event_logger.hpp"

struct logger_factory {
  logger_factory() : pipe(logger, 0) {}

  test_event_logger logger;
  log::pipe pipe;
};

// Ignore warnings from MSVC about unsafe getenv.
#if defined(_MSC_VER) && !defined(__clang__)
#  pragma warning(push)
#  pragma warning(disable:4996)
#endif

std::string test_data(const std::string &file) {
  auto path = std::getenv("TEST_DATA");
  std::string result = path ? path + pathsep + file : file;
#ifdef _WIN32
  result += ".exe";
#endif
  return result;
}

#if defined(_MSC_VER) && !defined(__clang__)
#  pragma warning(pop)
#endif

auto passed(bool expected) {
  return filter([](auto &&x) { return x.passed; }, equal_to(expected),
                "passed: ");
}

suite<> test_run_file("run files", [](auto &_) {
  using namespace platform;

  subsuite<logger_factory>(_, "run_test_file()", [](auto &_) {
    _.test("passing file", [](logger_factory &f) {
      expect(run_test_file({test_data("test_pass")}, f.pipe), passed(true));
      expect(f.logger.events, array(
        "started_suite", "started_test", "passed_test", "ended_suite"
      ));
    });

    _.test("failing file", [](logger_factory &f) {
      expect(run_test_file({test_data("test_fail")}, f.pipe), passed(true));
      expect(f.logger.events, array(
        "started_suite", "started_test", "failed_test", "ended_suite"
      ));
    });

    _.test("aborting file", [](logger_factory &f) {
      expect(run_test_file({test_data("test_abort")}, f.pipe), passed(false));
      expect(f.logger.events, array());
    });
  });

  subsuite<test_event_logger>(_, "run_test_files()", [](auto &_) {
    _.test("passing file", [](test_event_logger &logger) {
      run_test_files({test_data("test_pass")}, logger);
      expect(logger.events, array(
        "started_run", "started_file", "started_suite", "started_test",
        "passed_test", "ended_suite", "ended_file", "ended_run"
      ));
    });

    _.test("failing file", [](test_event_logger &logger) {
      run_test_files({test_data("test_fail")}, logger);
      expect(logger.events, array(
        "started_run", "started_file", "started_suite", "started_test",
        "failed_test", "ended_suite", "ended_file", "ended_run"
      ));
    });

    _.test("aborting file", [](test_event_logger &logger) {
      run_test_files({test_data("test_abort")}, logger);
      expect(logger.events, array(
        "started_run", "started_file", "failed_file", "ended_run"
      ));
    });

    _.test("multiple files", [](test_event_logger &logger) {
      run_test_files({
        test_data("test_pass"), test_data("test_fail"), test_data("test_abort")
      }, logger);
      expect(logger.events, array(
        "started_run",
          "started_file",
            "started_suite", "started_test", "passed_test", "ended_suite",
          "ended_file",
          "started_file",
            "started_suite", "started_test", "failed_test", "ended_suite",
          "ended_file",
          "started_file", "failed_file",
        "ended_run"
      ));
    });

    _.test("multiple runs", [](test_event_logger &logger) {
      for(int i = 0; i != 2; i++) {
        run_test_files({
          test_data("test_pass"), test_data("test_fail"),
          test_data("test_abort")
        }, logger);
      }

      expect(logger.events, array(
        "started_run",
          "started_file",
            "started_suite", "started_test", "passed_test", "ended_suite",
          "ended_file",
          "started_file",
            "started_suite", "started_test", "failed_test", "ended_suite",
          "ended_file",
          "started_file", "failed_file",
        "ended_run",
        "started_run",
          "started_file",
            "started_suite", "started_test", "passed_test", "ended_suite",
          "ended_file",
          "started_file",
            "started_suite", "started_test", "failed_test", "ended_suite",
          "ended_file",
          "started_file", "failed_file",
        "ended_run"
      ));
      expect(logger.files.size(), equal_to(3));
      expect(logger.tests.size(), equal_to(2));
    });
  });
});
