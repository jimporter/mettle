#include <mettle.hpp>
using namespace mettle;

#include <mettle/driver/log/summary.hpp>
#include <mettle/driver/log/indent.hpp>

#include "log_runs.hpp"

struct logger_factory {
  logger_factory(bool show_time, bool show_terminal)
    : is(ss), logger(is, nullptr, show_time, show_terminal) {}

  std::ostringstream ss;
  indenting_ostream is;
  log::summary logger;
};

suite<> test_summary("summary logger", [](auto &_) {
  subsuite<logger_factory>(_, "simple", bind_factory(false, false),
                           [](auto &_) {
    _.test("passing run", [](logger_factory &f) {
      passing_run(f.logger);
      f.logger.summarize();
      expect(f.ss.str(), equal_to(
        "3/3 tests passed\n"
      ));
    });

    _.test("failing run", [](logger_factory &f) {
      failing_run(f.logger);
      f.logger.summarize();
      expect(f.ss.str(), equal_to(
        "1/3 tests passed (1 skipped)\n"
        "  suite > subsuite > test SKIPPED\n"
        "    message\n"
        "    more\n"
        "  second suite > test FAILED\n"
        "    error\n"
        "    more\n"
      ));
    });

    _.test("failing file run", [](logger_factory &f) {
      failing_file_run(f.logger);
      f.logger.summarize();
      expect(f.ss.str(), equal_to(
        "1/3 tests passed (1 skipped) [1 file FAILED]\n"
        "  suite > subsuite > test SKIPPED\n"
        "    message\n"
        "    more\n"
        "  second suite > test FAILED\n"
        "    error\n"
        "    more\n"
        "  `test_file` FAILED\n"
        "    error\n"
        "    more\n"
      ));
    });
  });

  subsuite<logger_factory>(_, "multi-run", bind_factory(false, false),
                           [](auto &_) {
    _.test("passing runs", [](logger_factory &f) {
      passing_run(f.logger);
      passing_run(f.logger);
      f.logger.summarize();
      expect(f.ss.str(), equal_to(
        "3/3 tests passed\n"
      ));
    });

    _.test("failing runs", [](logger_factory &f) {
      failing_run(f.logger);
      failing_run(f.logger);
      f.logger.summarize();
      expect(f.ss.str(), equal_to(
        "1/3 tests passed (1 skipped)\n"
        "  suite > subsuite > test SKIPPED\n"
        "    message\n"
        "    more\n"
        "  second suite > test FAILED [2/2]\n"
        "    [#1] error\n"
        "         more\n"
        "    [#2] error\n"
        "         more\n"
      ));
    });

    _.test("failing file runs", [](logger_factory &f) {
      failing_file_run(f.logger);
      failing_file_run(f.logger);
      f.logger.summarize();
      expect(f.ss.str(), equal_to(
        "1/3 tests passed (1 skipped) [1 file FAILED]\n"
        "  suite > subsuite > test SKIPPED\n"
        "    message\n"
        "    more\n"
        "  second suite > test FAILED [2/2]\n"
        "    [#1] error\n"
        "         more\n"
        "    [#2] error\n"
        "         more\n"
        "  `test_file` FAILED [2/2]\n"
        "    [#1] error\n"
        "         more\n"
        "    [#2] error\n"
        "         more\n"
      ));
    });

    _.test("passing + failing runs", [](logger_factory &f) {
      passing_run(f.logger);
      failing_run(f.logger);
      failing_file_run(f.logger);
      f.logger.summarize();
      expect(f.ss.str(), equal_to(
        "1/3 tests passed (1 skipped) [1 file FAILED]\n"
        "  suite > subsuite > test SKIPPED\n"
        "    message\n"
        "    more\n"
        "  second suite > test FAILED [2/3]\n"
        "    [#2] error\n"
        "         more\n"
        "    [#3] error\n"
        "         more\n"
        "  `test_file` FAILED [1/3]\n"
        "    [#3] error\n"
        "         more\n"
      ));
    });
  });

  subsuite<logger_factory>(_, "show time", bind_factory(true, false),
                           [](auto &_) {
    _.test("passing run", [](logger_factory &f) {
      passing_run(f.logger);
      f.logger.summarize();
      expect(f.ss.str(), regex_match(
        "3/3 tests passed \\(took [\\d\\.]+ s\\)\n"
      ));
    });

    _.test("failing run", [](logger_factory &f) {
      failing_run(f.logger);
      f.logger.summarize();
      expect(f.ss.str(), regex_match(
        "1/3 tests passed \\(1 skipped\\) \\(took [\\d\\.]+ s\\)\n"
        "  suite > subsuite > test SKIPPED\n"
        "    message\n"
        "    more\n"
        "  second suite > test FAILED\n"
        "    error\n"
        "    more\n"
      ));
    });

    _.test("failing file run", [](logger_factory &f) {
      failing_file_run(f.logger);
      f.logger.summarize();
      expect(f.ss.str(), regex_match(
        "1/3 tests passed \\(1 skipped\\) \\[1 file FAILED\\] "
          "\\(took [\\d\\.]+ s\\)\n"
        "  suite > subsuite > test SKIPPED\n"
        "    message\n"
        "    more\n"
        "  second suite > test FAILED\n"
        "    error\n"
        "    more\n"
        "  `test_file` FAILED\n"
        "    error\n"
        "    more\n"
      ));
    });
  });

  subsuite<logger_factory>(_, "show terminal", bind_factory(false, true),
                           [](auto &_) {
    _.test("passing run", [](logger_factory &f) {
      passing_run(f.logger);
      f.logger.summarize();
      expect(f.ss.str(), equal_to(
        "3/3 tests passed\n"
      ));
    });

    _.test("failing run", [](logger_factory &f) {
      failing_run(f.logger);
      f.logger.summarize();
      expect(f.ss.str(), equal_to(
        "1/3 tests passed (1 skipped)\n"
        "  suite > subsuite > test SKIPPED\n"
        "    message\n"
        "    more\n"
        "  second suite > test FAILED\n"
        "    error\n"
        "    more\n"
        "\n"
        "    stdout:\n"
        "    standard output\n"
        "    stderr:\n"
        "    standard error\n"
      ));
    });

    _.test("failing file run", [](logger_factory &f) {
      failing_file_run(f.logger);
      f.logger.summarize();
      expect(f.ss.str(), equal_to(
        "1/3 tests passed (1 skipped) [1 file FAILED]\n"
        "  suite > subsuite > test SKIPPED\n"
        "    message\n"
        "    more\n"
        "  second suite > test FAILED\n"
        "    error\n"
        "    more\n"
        "\n"
        "    stdout:\n"
        "    standard output\n"
        "    stderr:\n"
        "    standard error\n"
        "  `test_file` FAILED\n"
        "    error\n"
        "    more\n"
      ));
    });
  });

});
