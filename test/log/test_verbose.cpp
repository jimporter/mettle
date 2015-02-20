#include <mettle.hpp>
using namespace mettle;

#include <mettle/driver/log/verbose.hpp>
#include <mettle/driver/log/indent.hpp>

struct logger_factory {
  logger_factory(std::size_t runs, bool show_time, bool show_terminal)
    : is(ss), logger(is, runs, show_time, show_terminal) {}

  std::ostringstream ss;
  indenting_ostream is;
  log::verbose logger;
};

using namespace std::literals::chrono_literals;

void full_run(log::verbose &logger) {
  log::test_output output = {"standard output", "standard error"};
  std::vector<std::string> suites = {"suite"};

  logger.started_run();
  logger.started_suite(suites);
  logger.started_test({suites, "test", 1});
  logger.passed_test({suites, "test", 1}, output, 100ms);

  suites.push_back("subsuite");
  logger.started_suite(suites);
  logger.started_test({suites, "test", 2});
  logger.skipped_test({suites, "test", 2}, "message");
  logger.failed_file("test_file", "error");

  suites = {"second suite"};
  logger.started_suite(suites);
  logger.started_test({suites, "test", 3});
  logger.failed_test({suites, "test", 3}, "error", output, 100ms);
  logger.ended_suite(suites);

  logger.ended_run();
}

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
      f.logger.started_suite({"suite"});
      expect(f.ss.str(), equal_to("suite\n"));
    });

    _.test("ended_suite()", [](logger_factory &f) {
      f.logger.ended_suite({"suite"});
      expect(f.ss.str(), equal_to(""));
    });

    _.test("started_test()", [](logger_factory &f) {
      f.logger.started_test({{"suite"}, "test", 1});
      expect(f.ss.str(), equal_to("test "));
    });

    _.test("passed_test()", [](logger_factory &f) {
      f.logger.passed_test({{"suite"}, "test", 1}, {}, 0ms);
      expect(f.ss.str(), equal_to("PASSED\n"));
    });

    _.test("failed_test()", [](logger_factory &f) {
      f.logger.failed_test({{"suite"}, "test", 1}, "error", {}, 0ms);
      expect(f.ss.str(), equal_to("FAILED\n  error\n"));
    });

    _.test("skipped_test()", [](logger_factory &f) {
      f.logger.skipped_test({{"suite"}, "test", 1}, "message");
      expect(f.ss.str(), equal_to("SKIPPED\n  message\n"));
    });

    _.test("started_file()", [](logger_factory &f) {
      f.logger.started_file("test_file");
      expect(f.ss.str(), equal_to(""));
    });

    _.test("ended_file()", [](logger_factory &f) {
      f.logger.ended_file("test_file");
      expect(f.ss.str(), equal_to(""));
    });

    _.test("failed_file()", [](logger_factory &f) {
      f.logger.failed_file("test_file", "error");
      expect(f.ss.str(), equal_to("`test_file` FAILED\n  error\n"));
    });

    _.test("full run", [](logger_factory &f) {
      full_run(f.logger);
      expect(f.ss.str(), equal_to(
        "suite\n"
        "  test PASSED\n"
        "\n"
        "  subsuite\n"
        "    test SKIPPED\n"
        "      message\n"
        "\n"
        "`test_file` FAILED\n"
        "  error\n"
        "\n"
        "second suite\n"
        "  test FAILED\n"
        "    error\n"
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

    _.test("full run", [](logger_factory &f) {
      full_run(f.logger);
      expect(f.ss.str(), equal_to(
        "Test run [#1/2]\n"
        "\n"
        "  suite\n"
        "    test PASSED\n"
        "\n"
        "    subsuite\n"
        "      test SKIPPED\n"
        "        message\n"
        "\n"
        "  `test_file` FAILED\n"
        "    error\n"
        "\n"
        "  second suite\n"
        "    test FAILED\n"
        "      error\n"
      ));
    });
  });

  subsuite<logger_factory>(_, "show time", bind_factory(1, true, false),
                           [](auto &_) {
    _.test("passed_test()", [](logger_factory &f) {
      f.logger.passed_test({{"suite"}, "test", 1}, {}, 100ms);
      expect(f.ss.str(), equal_to("PASSED (100 ms)\n"));
    });

    _.test("failed_test()", [](logger_factory &f) {
      f.logger.failed_test({{"suite"}, "test", 1}, "error", {}, 100ms);
      expect(f.ss.str(), equal_to("FAILED (100 ms)\n  error\n"));
    });

    _.test("full run", [](logger_factory &f) {
      full_run(f.logger);
      expect(f.ss.str(), equal_to(
        "suite\n"
        "  test PASSED (100 ms)\n"
        "\n"
        "  subsuite\n"
        "    test SKIPPED\n"
        "      message\n"
        "\n"
        "`test_file` FAILED\n"
        "  error\n"
        "\n"
        "second suite\n"
        "  test FAILED (100 ms)\n"
        "    error\n"
      ));
    });
  });

  subsuite<logger_factory>(_, "show terminal", bind_factory(1, false, true),
                           [](auto &_) {
    _.test("passed_test()", [](logger_factory &f) {
      f.logger.passed_test({{"suite"}, "test", 1}, {"foo", "bar"}, 0ms);
      expect(f.ss.str(), equal_to(
        "PASSED\n  stdout:\n  foo\n  stderr:\n  bar\n"
      ));
    });

    _.test("failed_test()", [](logger_factory &f) {
      f.logger.failed_test({{"suite"}, "test", 1}, "error", {"foo", "bar"},
                           0ms);
      expect(f.ss.str(), equal_to(
        "FAILED\n  error\n\n  stdout:\n  foo\n  stderr:\n  bar\n"
      ));
    });

    _.test("full run", [](logger_factory &f) {
      full_run(f.logger);
      expect(f.ss.str(), equal_to(
        "suite\n"
        "  test PASSED\n"
        "    stdout:\n"
        "    standard output\n"
        "    stderr:\n"
        "    standard error\n"
        "\n"
        "  subsuite\n"
        "    test SKIPPED\n"
        "      message\n"
        "\n"
        "`test_file` FAILED\n"
        "  error\n"
        "\n"
        "second suite\n"
        "  test FAILED\n"
        "    error\n"
        "\n"
        "    stdout:\n"
        "    standard output\n"
        "    stderr:\n"
        "    standard error\n"
      ));
    });
  });
});
