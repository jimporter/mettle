#include <mettle.hpp>
using namespace mettle;

#include <cstdlib>

#include "../../src/mettle/log_pipe.hpp"
#include "../../src/mettle/run_test_files.hpp"

#ifndef _WIN32
#  include "../../src/mettle/posix/run_test_file.hpp"
namespace platform = mettle::posix;
#else
#  include "../../src/mettle/windows/run_test_file.hpp"
namespace platform = mettle::windows;
#endif

#include "../test_event_logger.hpp"

struct logger_factory {
  logger_factory() : pipe(logger) {}

  test_event_logger logger;
  log::pipe pipe;
};

std::string test_data(const std::string &file) {
  auto path = getenv("TEST_DATA");
  if(!path)
    return file;
  return path + file;
}

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
  });
});
