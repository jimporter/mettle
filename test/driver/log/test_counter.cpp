#include <mettle.hpp>
using namespace mettle;

#include <mettle/driver/log/counter.hpp>
#include <mettle/driver/log/indent.hpp>

#include "log_runs.hpp"

struct logger_factory {
  logger_factory() : is(ss), logger(is) {}

  std::ostringstream ss;
  indenting_ostream is;
  log::counter logger;
};

using namespace std::literals::chrono_literals;

suite<logger_factory> test_verbose("counter logger", [](auto &_) {
  _.test("started_run()", [](logger_factory &f) {
    f.logger.started_run();
    expect(f.ss.str(), equal_to("\r[   0 |   0 |   0 |   0 ]"));
  });

  _.test("ended_run()", [](logger_factory &f) {
    f.logger.ended_run();
    expect(f.ss.str(), equal_to("\n"));
  });

  _.test("started_suite()", [](logger_factory &f) {
    f.logger.started_suite({"suite"});
    expect(f.ss.str(), equal_to(""));
  });

  _.test("ended_suite()", [](logger_factory &f) {
    f.logger.ended_suite({"suite"});
    expect(f.ss.str(), equal_to(""));
  });

  _.test("started_test()", [](logger_factory &f) {
    f.logger.started_test({1, {"suite"}, "test", "file.cpp", 10});
    expect(f.ss.str(), equal_to(""));
  });

  _.test("passed_test()", [](logger_factory &f) {
    f.logger.passed_test({1, {"suite"}, "test", "file.cpp", 10}, {}, 0ms);
    expect(f.ss.str(), equal_to("\r[   1 |   1 |   0 |   0 ]"));
  });

  _.test("failed_test()", [](logger_factory &f) {
    f.logger.failed_test({1, {"suite"}, "test", "file.cpp", 10}, "error", {},
                         0ms);
    expect(f.ss.str(), equal_to("\r[   1 |   0 |   0 |   1 ]"));
  });

  _.test("skipped_test()", [](logger_factory &f) {
    f.logger.skipped_test({1, {"suite"}, "test", "file.cpp", 10}, "message");
    expect(f.ss.str(), equal_to("\r[   1 |   0 |   1 |   0 ]"));
  });

  _.test("started_file()", [](logger_factory &f) {
    f.logger.started_file({"test_file", 100});
    expect(f.ss.str(), equal_to(""));
  });

  _.test("ended_file()", [](logger_factory &f) {
    f.logger.ended_file({"test_file", 100});
    expect(f.ss.str(), equal_to(""));
  });

  _.test("failed_file()", [](logger_factory &f) {
    f.logger.failed_file({"test_file", 100}, "error");
    expect(f.ss.str(), equal_to("\r[   1 |   0 |   0 |   1 ]"));
  });

  _.test("passing run", [](logger_factory &f) {
    passing_run(f.logger);
    expect(f.ss.str(), equal_to(
      "\r[   0 |   0 |   0 |   0 ]"
      "\r[   1 |   1 |   0 |   0 ]"
      "\r[   2 |   2 |   0 |   0 ]"
      "\r[   3 |   3 |   0 |   0 ]"
      "\r[   4 |   4 |   0 |   0 ]\n"
    ));
  });

  _.test("failing run", [](logger_factory &f) {
    failing_run(f.logger);
    expect(f.ss.str(), equal_to(
      "\r[   0 |   0 |   0 |   0 ]"
      "\r[   1 |   1 |   0 |   0 ]"
      "\r[   2 |   1 |   0 |   1 ]"
      "\r[   3 |   1 |   1 |   1 ]"
      "\r[   4 |   1 |   1 |   2 ]\n"
    ));
  });

  _.test("failing file run", [](logger_factory &f) {
    failing_file_run(f.logger);
    expect(f.ss.str(), equal_to(
      "\r[   0 |   0 |   0 |   0 ]"
      "\r[   1 |   1 |   0 |   0 ]"
      "\r[   2 |   2 |   0 |   0 ]"
      "\r[   3 |   2 |   1 |   0 ]"
      "\r[   4 |   2 |   1 |   1 ]"
      "\r[   5 |   3 |   1 |   1 ]\n"
    ));
  });

  _.test("failing test and file run", [](logger_factory &f) {
    failing_test_and_file_run(f.logger);
    expect(f.ss.str(), equal_to(
      "\r[   0 |   0 |   0 |   0 ]"
      "\r[   1 |   1 |   0 |   0 ]"
      "\r[   2 |   1 |   0 |   1 ]"
      "\r[   3 |   1 |   1 |   1 ]"
      "\r[   4 |   1 |   1 |   2 ]"
      "\r[   5 |   1 |   1 |   3 ]\n"
    ));
  });

  _.test("multiple runs", [](logger_factory &f) {
    passing_run(f.logger);
    failing_run(f.logger);
    expect(f.ss.str(), equal_to(
      "\r[   0 |   0 |   0 |   0 ]"
      "\r[   1 |   1 |   0 |   0 ]"
      "\r[   2 |   2 |   0 |   0 ]"
      "\r[   3 |   3 |   0 |   0 ]"
      "\r[   4 |   4 |   0 |   0 ]\n"
      "\r[   0 |   0 |   0 |   0 ]"
      "\r[   1 |   1 |   0 |   0 ]"
      "\r[   2 |   1 |   0 |   1 ]"
      "\r[   3 |   1 |   1 |   1 ]"
      "\r[   4 |   1 |   1 |   2 ]\n"
    ));
  });
});
