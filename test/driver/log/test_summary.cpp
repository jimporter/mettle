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
      expect(f.logger.good(), equal_to(true));
      expect(f.ss.str(), equal_to(
        "4/4 tests passed\n"
      ));
    });

    _.test("failing run", [](logger_factory &f) {
      failing_run(f.logger);
      f.logger.summarize();
      expect(f.logger.good(), equal_to(false));
      expect(f.ss.str(), equal_to(
        "1/4 tests passed (1 skipped)\n"
        "  suite > test 2 FAILED\n"
        "    error\n"
        "  suite > subsuite > test 3 SKIPPED\n"
        "    message\n"
        "    more\n"
        "  second suite > test 4 FAILED\n"
        "    error\n"
        "    more\n"
      ));
    });

    _.test("failing file run", [](logger_factory &f) {
      failing_file_run(f.logger);
      f.logger.summarize();
      expect(f.logger.good(), equal_to(false));
      expect(f.ss.str(), equal_to(
        "3/4 tests passed (1 skipped) [1 file FAILED]\n"
        "  suite > subsuite > test 3 SKIPPED\n"
        "    message\n"
        "    more\n"
        "  `file.cpp` FAILED\n"
        "    error\n"
        "    more\n"
      ));
    });

    _.test("failing test and file run", [](logger_factory &f) {
      failing_test_and_file_run(f.logger);
      f.logger.summarize();
      expect(f.logger.good(), equal_to(false));
      expect(f.ss.str(), equal_to(
        "1/4 tests passed (1 skipped) [1 file FAILED]\n"
        "  suite > test 2 FAILED\n"
        "    error\n"
        "  suite > subsuite > test 3 SKIPPED\n"
        "    message\n"
        "    more\n"
        "  `file.cpp` FAILED\n"
        "    error\n"
        "    more\n"
        "  second suite > test 4 FAILED\n"
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
      expect(f.logger.good(), equal_to(true));
      expect(f.ss.str(), equal_to(
        "4/4 tests passed\n"
      ));
    });

    _.test("failing runs", [](logger_factory &f) {
      failing_run(f.logger);
      failing_run(f.logger);
      f.logger.summarize();
      expect(f.logger.good(), equal_to(false));
      expect(f.ss.str(), equal_to(
        "1/4 tests passed (1 skipped)\n"
        "  suite > test 2 FAILED [2/2]\n"
        "    [#1] error\n"
        "    [#2] error\n"
        "  suite > subsuite > test 3 SKIPPED\n"
        "    message\n"
        "    more\n"
        "  second suite > test 4 FAILED [2/2]\n"
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
      expect(f.logger.good(), equal_to(false));
      expect(f.ss.str(), equal_to(
        "3/4 tests passed (1 skipped) [1 file FAILED]\n"
        "  suite > subsuite > test 3 SKIPPED\n"
        "    message\n"
        "    more\n"
        "  `file.cpp` FAILED [2/2]\n"
        "    [#1] error\n"
        "         more\n"
        "    [#2] error\n"
        "         more\n"
      ));
    });

    _.test("failing test and file runs", [](logger_factory &f) {
      failing_test_and_file_run(f.logger);
      failing_test_and_file_run(f.logger);
      f.logger.summarize();
      expect(f.logger.good(), equal_to(false));
      expect(f.ss.str(), equal_to(
        "1/4 tests passed (1 skipped) [1 file FAILED]\n"
        "  suite > test 2 FAILED [2/2]\n"
        "    [#1] error\n"
        "    [#2] error\n"
        "  suite > subsuite > test 3 SKIPPED\n"
        "    message\n"
        "    more\n"
        "  `file.cpp` FAILED [2/2]\n"
        "    [#1] error\n"
        "         more\n"
        "    [#2] error\n"
        "         more\n"
        "  second suite > test 4 FAILED [2/2]\n"
        "    [#1] error\n"
        "         more\n"
        "    [#2] error\n"
        "         more\n"
      ));
    });

    _.test("passing + failing runs", [](logger_factory &f) {
      passing_run(f.logger);
      failing_run(f.logger);
      failing_test_and_file_run(f.logger);
      f.logger.summarize();
      expect(f.logger.good(), equal_to(false));
      expect(f.ss.str(), equal_to(
        "1/4 tests passed (1 skipped) [1 file FAILED]\n"
        "  suite > test 2 FAILED [2/3]\n"
        "    [#2] error\n"
        "    [#3] error\n"
        "  suite > subsuite > test 3 SKIPPED\n"
        "    message\n"
        "    more\n"
        "  `file.cpp` FAILED [1/3]\n"
        "    [#3] error\n"
        "         more\n"
        "  second suite > test 4 FAILED [2/3]\n"
        "    [#2] error\n"
        "         more\n"
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
      expect(f.logger.good(), equal_to(true));
      expect(f.ss.str(), regex_match(
        "4/4 tests passed \\(took [\\d\\.]+ s\\)\n"
      ));
    });

    _.test("failing run", [](logger_factory &f) {
      failing_run(f.logger);
      f.logger.summarize();
      expect(f.logger.good(), equal_to(false));
      expect(f.ss.str(), regex_match(
        "1/4 tests passed \\(1 skipped\\) \\(took [\\d\\.]+ s\\)\n"
        "  suite > test 2 FAILED\n"
        "    error\n"
        "  suite > subsuite > test 3 SKIPPED\n"
        "    message\n"
        "    more\n"
        "  second suite > test 4 FAILED\n"
        "    error\n"
        "    more\n"
      ));
    });

    _.test("failing file run", [](logger_factory &f) {
      failing_file_run(f.logger);
      f.logger.summarize();
      expect(f.logger.good(), equal_to(false));
      expect(f.ss.str(), regex_match(
        "3/4 tests passed \\(1 skipped\\) \\[1 file FAILED\\] "
          "\\(took [\\d\\.]+ s\\)\n"
        "  suite > subsuite > test 3 SKIPPED\n"
        "    message\n"
        "    more\n"
        "  `file.cpp` FAILED\n"
        "    error\n"
        "    more\n"
      ));
    });

    _.test("failing test and file run", [](logger_factory &f) {
      failing_test_and_file_run(f.logger);
      f.logger.summarize();
      expect(f.logger.good(), equal_to(false));
      expect(f.ss.str(), regex_match(
        "1/4 tests passed \\(1 skipped\\) \\[1 file FAILED\\] "
          "\\(took [\\d\\.]+ s\\)\n"
        "  suite > test 2 FAILED\n"
        "    error\n"
        "  suite > subsuite > test 3 SKIPPED\n"
        "    message\n"
        "    more\n"
        "  `file.cpp` FAILED\n"
        "    error\n"
        "    more\n"
        "  second suite > test 4 FAILED\n"
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
      expect(f.logger.good(), equal_to(true));
      expect(f.ss.str(), equal_to(
        "4/4 tests passed\n"
      ));
    });

    _.test("failing run", [](logger_factory &f) {
      failing_run(f.logger);
      f.logger.summarize();
      expect(f.logger.good(), equal_to(false));
      expect(f.ss.str(), equal_to(
        "1/4 tests passed (1 skipped)\n"
        "  suite > test 2 FAILED\n"
        "    error\n"
        "  suite > subsuite > test 3 SKIPPED\n"
        "    message\n"
        "    more\n"
        "  second suite > test 4 FAILED\n"
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
      expect(f.logger.good(), equal_to(false));
      expect(f.ss.str(), equal_to(
        "3/4 tests passed (1 skipped) [1 file FAILED]\n"
        "  suite > subsuite > test 3 SKIPPED\n"
        "    message\n"
        "    more\n"
        "  `file.cpp` FAILED\n"
        "    error\n"
        "    more\n"
      ));
    });

    _.test("failing test and file run", [](logger_factory &f) {
      failing_test_and_file_run(f.logger);
      f.logger.summarize();
      expect(f.logger.good(), equal_to(false));
      expect(f.ss.str(), equal_to(
        "1/4 tests passed (1 skipped) [1 file FAILED]\n"
        "  suite > test 2 FAILED\n"
        "    error\n"
        "  suite > subsuite > test 3 SKIPPED\n"
        "    message\n"
        "    more\n"
        "  `file.cpp` FAILED\n"
        "    error\n"
        "    more\n"
        "  second suite > test 4 FAILED\n"
        "    error\n"
        "    more\n"
        "\n"
        "    stdout:\n"
        "    standard output\n"
        "    stderr:\n"
        "    standard error\n"
      ));
    });
  });

});
