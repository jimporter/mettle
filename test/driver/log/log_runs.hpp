#ifndef INC_METTLE_TEST_DRIVER_LOG_LOG_RUNS_HPP
#define INC_METTLE_TEST_DRIVER_LOG_LOG_RUNS_HPP

#include <mettle/driver/log/core.hpp>

inline void passing_run(log::test_logger &logger) {
  using namespace std::literals::chrono_literals;

  log::test_output output = {"standard output", "standard error"};
  std::vector<std::string> suites = {"suite"};

  logger.started_run();

  logger.started_suite(suites);
  logger.started_test({suites, "test", 1});
  logger.passed_test({suites, "test", 1}, output, 100ms);

  suites.push_back("subsuite");
  logger.started_suite(suites);
  logger.started_test({suites, "test", 2});
  logger.passed_test({suites, "test", 2}, output, 100ms);
  logger.ended_suite(suites);

  logger.ended_suite(suites);

  suites = {"second suite"};
  logger.started_suite(suites);
  logger.started_test({suites, "test", 3});
  logger.passed_test({suites, "test", 3}, output, 100ms);
  logger.ended_suite(suites);

  logger.ended_run();
}

inline void failing_run(log::test_logger &logger) {
  using namespace std::literals::chrono_literals;

  log::test_output output = {"standard output", "standard error"};
  std::vector<std::string> suites = {"suite"};

  logger.started_run();

  logger.started_suite(suites);
  logger.started_test({suites, "test", 1});
  logger.passed_test({suites, "test", 1}, output, 100ms);

  suites.push_back("subsuite");
  logger.started_suite(suites);
  logger.started_test({suites, "test", 2});
  logger.skipped_test({suites, "test", 2}, "message\nmore");
  logger.ended_suite(suites);

  logger.ended_suite(suites);

  suites = {"second suite"};
  logger.started_suite(suites);
  logger.started_test({suites, "test", 3});
  logger.failed_test({suites, "test", 3}, "error\nmore", output, 100ms);
  logger.ended_suite(suites);

  logger.ended_run();
}

inline void failing_file_run(log::file_logger &logger) {
  using namespace std::literals::chrono_literals;

  log::test_output output = {"standard output", "standard error"};
  std::vector<std::string> suites = {"suite"};

  logger.started_run();

  logger.started_suite(suites);
  logger.started_test({suites, "test", 1});
  logger.passed_test({suites, "test", 1}, output, 100ms);

  suites.push_back("subsuite");
  logger.started_suite(suites);
  logger.started_test({suites, "test", 2});
  logger.skipped_test({suites, "test", 2}, "message\nmore");
  logger.failed_file("test_file", "error\nmore");

  suites = {"second suite"};
  logger.started_suite(suites);
  logger.started_test({suites, "test", 3});
  logger.failed_test({suites, "test", 3}, "error\nmore", output, 100ms);
  logger.ended_suite(suites);

  logger.ended_run();
}

#endif
