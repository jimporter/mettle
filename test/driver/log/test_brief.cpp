#include <mettle.hpp>
using namespace mettle;

#include <mettle/driver/log/brief.hpp>
#include <mettle/driver/log/indent.hpp>

#include "log_runs.hpp"

struct logger_factory {
  logger_factory() : is(ss), logger(is) {}
  std::ostringstream ss;
  indenting_ostream is;
  log::brief logger;
};

suite<logger_factory> test_brief("brief logger", [](auto &_) {
  _.test("started_run()", [](logger_factory &f) {
    f.logger.started_run();
    expect(f.ss.str(), equal_to(""));
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
    f.logger.started_test({{"suite"}, "test", 1});
    expect(f.ss.str(), equal_to(""));
  });

  _.test("passed_test()", [](logger_factory &f) {
    f.logger.passed_test({{"suite"}, "test", 1}, {}, {});
    expect(f.ss.str(), equal_to("."));
  });

  _.test("failed_test()", [](logger_factory &f) {
    f.logger.failed_test({{"suite"}, "test", 1}, "error", {}, {});
    expect(f.ss.str(), equal_to("!"));
  });

  _.test("skipped_test()", [](logger_factory &f) {
    f.logger.skipped_test({{"suite"}, "test", 1}, "message");
    expect(f.ss.str(), equal_to("_"));
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
    expect(f.ss.str(), equal_to("X"));
  });

  _.test("passing run", [](logger_factory &f) {
    passing_run(f.logger);
    expect(f.ss.str(), equal_to("....\n"));
  });

  _.test("failing run", [](logger_factory &f) {
    failing_run(f.logger);
    expect(f.ss.str(), equal_to(".!_!\n"));
  });

  _.test("failing file run", [](logger_factory &f) {
    failing_file_run(f.logger);
    expect(f.ss.str(), equal_to(".._X.\n"));
  });

  _.test("failing test and file run", [](logger_factory &f) {
    failing_test_and_file_run(f.logger);
    expect(f.ss.str(), equal_to(".!_X!\n"));
  });
});
