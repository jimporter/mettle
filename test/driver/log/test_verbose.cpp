#include <mettle.hpp>
using namespace mettle;

#include <mettle/driver/log/verbose.hpp>
#include <mettle/driver/log/indent.hpp>

#include "log_runs.hpp"

struct logger_factory {
  logger_factory(std::size_t runs, bool show_time, bool show_terminal)
    : is(ss), logger(is, runs, show_time, show_terminal) {}

  std::ostringstream ss;
  indenting_ostream is;
  log::verbose logger;
};

using namespace std::literals::chrono_literals;

suite<> test_verbose("verbose logger", [](auto &_) {
  subsuite<logger_factory>(_, "simple", bind_factory(1, false, false),
                           [](auto &_) {
    _.test("started_run()", [](logger_factory &f) {
      f.logger.started_run();
      expect(f.ss.str(), equal_to(""));
    });

    _.test("ended_run()", [](logger_factory &f) {
      f.logger.ended_run();
      expect(f.ss.str(), equal_to(""));
    });

    _.test("started_suite()", [](logger_factory &f) {
      f.logger.started_suite({{"suite", "file.cpp", 1}});
      expect(f.ss.str(), equal_to("suite\n"));
    });

    _.test("ended_suite()", [](logger_factory &f) {
      f.logger.ended_suite({{"suite", "file.cpp", 1}});
      expect(f.ss.str(), equal_to(""));
    });

    _.test("started_test()", [](logger_factory &f) {
      f.logger.started_test({
        1, {{"suite", "file.cpp", 1}}, "test", "file.cpp", 10
      });
      expect(f.ss.str(), equal_to("test "));
    });

    _.test("passed_test()", [](logger_factory &f) {
      f.logger.passed_test({
        1, {{"suite", "file.cpp", 1}}, "test", "file.cpp", 10
      }, {}, 0ms);
      expect(f.ss.str(), equal_to("PASSED\n"));
    });

    _.test("failed_test()", [](logger_factory &f) {
      f.logger.failed_test({
        1, {{"suite", "file.cpp", 1}}, "test", "file.cpp", 10
      }, "error", {}, 0ms);
      expect(f.ss.str(), equal_to("FAILED\n  error\n"));
    });

    _.test("skipped_test()", [](logger_factory &f) {
      f.logger.skipped_test({
        1, {{"suite", "file.cpp", 1}}, "test", "file.cpp", 10
      }, "message");
      expect(f.ss.str(), equal_to("SKIPPED\n  message\n"));
    });

    _.test("started_file()", [](logger_factory &f) {
      f.logger.started_file({100, "file.cpp"});
      expect(f.ss.str(), equal_to(""));
    });

    _.test("ended_file()", [](logger_factory &f) {
      f.logger.ended_file({100, "file.cpp"});
      expect(f.ss.str(), equal_to(""));
    });

    _.test("failed_file()", [](logger_factory &f) {
      f.logger.failed_file({100, "file.cpp"}, "error");
      expect(f.ss.str(), equal_to("`file.cpp` FAILED\n  error\n"));
    });

    _.test("passing run", [](logger_factory &f) {
      passing_run(f.logger);
      expect(f.ss.str(), equal_to(
        "suite\n"
        "  test 1 PASSED\n"
        "  test 2 PASSED\n"
        "\n"
        "  subsuite\n"
        "    test 3 PASSED [...]\n"
        "\n"
        "second suite\n"
        "  test 4 PASSED [...]\n"
      ));
    });

    _.test("failing run", [](logger_factory &f) {
      failing_run(f.logger);
      expect(f.ss.str(), equal_to(
        "suite\n"
        "  test 1 PASSED [...]\n"
        "  test 2 FAILED\n"
        "    error\n"
        "\n"
        "  subsuite\n"
        "    test 3 SKIPPED\n"
        "      message\n"
        "      more\n"
        "\n"
        "second suite\n"
        "  test 4 FAILED [...]\n"
        "    error\n"
        "    more\n"
      ));
    });

    _.test("failing file run", [](logger_factory &f) {
      failing_file_run(f.logger);
      expect(f.ss.str(), equal_to(
        "suite\n"
        "  test 1 PASSED\n"
        "  test 2 PASSED\n"
        "\n"
        "  subsuite\n"
        "    test 3 SKIPPED\n"
        "      message\n"
        "      more\n"
        "\n"
        "`file.cpp` FAILED\n"
        "  error\n"
        "  more\n"
        "\n"
        "second suite\n"
        "  test 4 PASSED [...]\n"
      ));
    });

    _.test("failing test and file run", [](logger_factory &f) {
      failing_test_and_file_run(f.logger);
      expect(f.ss.str(), equal_to(
        "suite\n"
        "  test 1 PASSED [...]\n"
        "  test 2 FAILED\n"
        "    error\n"
        "\n"
        "  subsuite\n"
        "    test 3 SKIPPED\n"
        "      message\n"
        "      more\n"
        "\n"
        "`file.cpp` FAILED\n"
        "  error\n"
        "  more\n"
        "\n"
        "second suite\n"
        "  test 4 FAILED [...]\n"
        "    error\n"
        "    more\n"
      ));
    });
  });

  subsuite<logger_factory>(_, "multi-run", bind_factory(2, false, false),
                           [](auto &_) {
    _.test("started_run()", [](logger_factory &f) {
      f.logger.started_run();
      expect(f.ss.str(), equal_to("Test run [#1/2]\n\n"));
    });

    _.test("ended_run()", [](logger_factory &f) {
      f.logger.ended_run();
      expect(f.ss.str(), equal_to(""));
    });

    _.test("passing run", [](logger_factory &f) {
      passing_run(f.logger);
      expect(f.ss.str(), equal_to(
        "Test run [#1/2]\n"
        "\n"
        "  suite\n"
        "    test 1 PASSED\n"
        "    test 2 PASSED\n"
        "\n"
        "    subsuite\n"
        "      test 3 PASSED [...]\n"
        "\n"
        "  second suite\n"
        "    test 4 PASSED [...]\n"
      ));
    });

    _.test("failing run", [](logger_factory &f) {
      failing_run(f.logger);
      expect(f.ss.str(), equal_to(
        "Test run [#1/2]\n"
        "\n"
        "  suite\n"
        "    test 1 PASSED [...]\n"
        "    test 2 FAILED\n"
        "      error\n"
        "\n"
        "    subsuite\n"
        "      test 3 SKIPPED\n"
        "        message\n"
        "        more\n"
        "\n"
        "  second suite\n"
        "    test 4 FAILED [...]\n"
        "      error\n"
        "      more\n"
      ));
    });

    _.test("failing file run", [](logger_factory &f) {
      failing_file_run(f.logger);
      expect(f.ss.str(), equal_to(
        "Test run [#1/2]\n"
        "\n"
        "  suite\n"
        "    test 1 PASSED\n"
        "    test 2 PASSED\n"
        "\n"
        "    subsuite\n"
        "      test 3 SKIPPED\n"
        "        message\n"
        "        more\n"
        "\n"
        "  `file.cpp` FAILED\n"
        "    error\n"
        "    more\n"
        "\n"
        "  second suite\n"
        "    test 4 PASSED [...]\n"
      ));
    });

    _.test("failing test and file run", [](logger_factory &f) {
      failing_test_and_file_run(f.logger);
      expect(f.ss.str(), equal_to(
        "Test run [#1/2]\n"
        "\n"
        "  suite\n"
        "    test 1 PASSED [...]\n"
        "    test 2 FAILED\n"
        "      error\n"
        "\n"
        "    subsuite\n"
        "      test 3 SKIPPED\n"
        "        message\n"
        "        more\n"
        "\n"
        "  `file.cpp` FAILED\n"
        "    error\n"
        "    more\n"
        "\n"
        "  second suite\n"
        "    test 4 FAILED [...]\n"
        "      error\n"
        "      more\n"
      ));
    });
  });

  subsuite<logger_factory>(_, "show time", bind_factory(1, true, false),
                           [](auto &_) {
    _.test("passed_test()", [](logger_factory &f) {
      f.logger.passed_test({
        1, {{"suite", "file.cpp", 1}}, "test", "file.cpp", 10
      }, {}, 100ms);
      expect(f.ss.str(), equal_to("PASSED (100 ms)\n"));
    });

    _.test("failed_test()", [](logger_factory &f) {
      f.logger.failed_test({
        1, {{"suite", "file.cpp", 1}}, "test", "file.cpp", 10
      }, "error", {}, 100ms);
      expect(f.ss.str(), equal_to("FAILED (100 ms)\n  error\n"));
    });

    _.test("passing run", [](logger_factory &f) {
      passing_run(f.logger);
      expect(f.ss.str(), equal_to(
        "suite\n"
        "  test 1 PASSED (100 ms)\n"
        "  test 2 PASSED (100 ms)\n"
        "\n"
        "  subsuite\n"
        "    test 3 PASSED [...] (100 ms)\n"
        "\n"
        "second suite\n"
        "  test 4 PASSED [...] (100 ms)\n"
      ));
    });

    _.test("failing run", [](logger_factory &f) {
      failing_run(f.logger);
      expect(f.ss.str(), equal_to(
        "suite\n"
        "  test 1 PASSED [...] (100 ms)\n"
        "  test 2 FAILED (100 ms)\n"
        "    error\n"
        "\n"
        "  subsuite\n"
        "    test 3 SKIPPED\n"
        "      message\n"
        "      more\n"
        "\n"
        "second suite\n"
        "  test 4 FAILED [...] (100 ms)\n"
        "    error\n"
        "    more\n"
      ));
    });

    _.test("failing file run", [](logger_factory &f) {
      failing_file_run(f.logger);
      expect(f.ss.str(), equal_to(
        "suite\n"
        "  test 1 PASSED (100 ms)\n"
        "  test 2 PASSED (100 ms)\n"
        "\n"
        "  subsuite\n"
        "    test 3 SKIPPED\n"
        "      message\n"
        "      more\n"
        "\n"
        "`file.cpp` FAILED\n"
        "  error\n"
        "  more\n"
        "\n"
        "second suite\n"
        "  test 4 PASSED [...] (100 ms)\n"
      ));
    });

    _.test("failing test and file run", [](logger_factory &f) {
      failing_test_and_file_run(f.logger);
      expect(f.ss.str(), equal_to(
        "suite\n"
        "  test 1 PASSED [...] (100 ms)\n"
        "  test 2 FAILED (100 ms)\n"
        "    error\n"
        "\n"
        "  subsuite\n"
        "    test 3 SKIPPED\n"
        "      message\n"
        "      more\n"
        "\n"
        "`file.cpp` FAILED\n"
        "  error\n"
        "  more\n"
        "\n"
        "second suite\n"
        "  test 4 FAILED [...] (100 ms)\n"
        "    error\n"
        "    more\n"
      ));
    });
  });

  subsuite<logger_factory>(_, "show terminal", bind_factory(1, false, true),
                           [](auto &_) {
    _.test("passed_test()", [](logger_factory &f) {
      f.logger.passed_test({
        1, {{"suite", "file.cpp", 1}}, "test", "file.cpp", 10
      }, {"foo", "bar"}, 0ms);
      expect(f.ss.str(), equal_to(
        "PASSED\n  stdout:\n  foo\n  stderr:\n  bar\n"
      ));
    });

    _.test("failed_test()", [](logger_factory &f) {
      f.logger.failed_test({
        1, {{"suite", "file.cpp", 1}}, "test", "file.cpp", 10
      }, "error", {"foo", "bar"}, 0ms);
      expect(f.ss.str(), equal_to(
        "FAILED\n  error\n\n  stdout:\n  foo\n  stderr:\n  bar\n"
      ));
    });

    _.test("passing run", [](logger_factory &f) {
      passing_run(f.logger);
      expect(f.ss.str(), equal_to(
        "suite\n"
        "  test 1 PASSED\n"
        "  test 2 PASSED\n"
        "\n"
        "  subsuite\n"
        "    test 3 PASSED\n"
        "      stdout:\n"
        "      standard output\n"
        "      stderr:\n"
        "      standard error\n"
        "\n"
        "second suite\n"
        "  test 4 PASSED\n"
        "    stdout:\n"
        "    standard output\n"
        "    stderr:\n"
        "    standard error\n"
      ));
    });

    _.test("failing run", [](logger_factory &f) {
      failing_run(f.logger);
      expect(f.ss.str(), equal_to(
        "suite\n"
        "  test 1 PASSED\n"
        "    stdout:\n"
        "    standard output\n"
        "    stderr:\n"
        "    standard error\n"
        "  test 2 FAILED\n"
        "    error\n"
        "\n"
        "  subsuite\n"
        "    test 3 SKIPPED\n"
        "      message\n"
        "      more\n"
        "\n"
        "second suite\n"
        "  test 4 FAILED\n"
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
      expect(f.ss.str(), equal_to(
        "suite\n"
        "  test 1 PASSED\n"
        "  test 2 PASSED\n"
        "\n"
        "  subsuite\n"
        "    test 3 SKIPPED\n"
        "      message\n"
        "      more\n"
        "\n"
        "`file.cpp` FAILED\n"
        "  error\n"
        "  more\n"
        "\n"
        "second suite\n"
        "  test 4 PASSED\n"
        "    stdout:\n"
        "    standard output\n"
        "    stderr:\n"
        "    standard error\n"
      ));
    });

    _.test("failing test and file run", [](logger_factory &f) {
      failing_test_and_file_run(f.logger);
      expect(f.ss.str(), equal_to(
        "suite\n"
        "  test 1 PASSED\n"
        "    stdout:\n"
        "    standard output\n"
        "    stderr:\n"
        "    standard error\n"
        "  test 2 FAILED\n"
        "    error\n"
        "\n"
        "  subsuite\n"
        "    test 3 SKIPPED\n"
        "      message\n"
        "      more\n"
        "\n"
        "`file.cpp` FAILED\n"
        "  error\n"
        "  more\n"
        "\n"
        "second suite\n"
        "  test 4 FAILED\n"
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
