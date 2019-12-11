#ifndef INC_METTLE_TEST_DRIVER_LOG_LOG_RUNS_HPP
#define INC_METTLE_TEST_DRIVER_LOG_LOG_RUNS_HPP

#include <mettle/test_uid.hpp>
#include <mettle/driver/log/core.hpp>

inline void passing_run(mettle::log::test_logger &logger) {
  using namespace std::literals::chrono_literals;

  mettle::log::test_output output = {"standard output", "standard error"};
  std::vector<std::string> suites = {"suite"};
  mettle::detail::file_uid_maker f;
  mettle::test_uid uid;

  logger.started_run();

  uid = f.make_file_uid();
  logger.started_suite(suites);
  logger.started_test({suites, "test 1", uid + 1});
  logger.passed_test({suites, "test 1", uid + 1}, log::test_output{}, 100ms);
  logger.started_test({suites, "test 2", uid + 2});
  logger.passed_test({suites, "test 2", uid + 2}, log::test_output{}, 100ms);

  suites.push_back("subsuite");
  logger.started_suite(suites);
  logger.started_test({suites, "test 3", uid + 3});
  logger.passed_test({suites, "test 3", uid + 3}, output, 100ms);
  logger.ended_suite(suites);

  logger.ended_suite(suites);

  uid = f.make_file_uid();
  suites = {"second suite"};
  logger.started_suite(suites);
  logger.started_test({suites, "test 4", uid + 1});
  logger.passed_test({suites, "test 4", uid + 1}, output, 100ms);
  logger.ended_suite(suites);

  logger.ended_run();
}

inline void failing_run(mettle::log::test_logger &logger) {
  using namespace std::literals::chrono_literals;

  mettle::log::test_output output = {"standard output", "standard error"};
  std::vector<std::string> suites = {"suite"};
  mettle::detail::file_uid_maker f;
  mettle::test_uid uid;

  logger.started_run();

  uid = f.make_file_uid();
  logger.started_suite(suites);
  logger.started_test({suites, "test 1", uid + 1});
  logger.passed_test({suites, "test 1", uid + 1}, output, 100ms);
  logger.started_test({suites, "test 2", uid + 2});
  logger.failed_test({suites, "test 2", uid + 2}, "error", log::test_output{},
                     100ms);

  suites.push_back("subsuite");
  logger.started_suite(suites);
  logger.started_test({suites, "test 3", uid + 3});
  logger.skipped_test({suites, "test 3", uid + 3}, "message\nmore");
  logger.ended_suite(suites);

  logger.ended_suite(suites);

  uid = f.make_file_uid();
  suites = {"second suite"};
  logger.started_suite(suites);
  logger.started_test({suites, "test 4", uid + 1});
  logger.failed_test({suites, "test 4", uid + 1}, "error\nmore", output, 100ms);
  logger.ended_suite(suites);

  logger.ended_run();
}

inline void failing_file_run(mettle::log::file_logger &logger) {
  using namespace std::literals::chrono_literals;

  mettle::log::test_output output = {"standard output", "standard error"};
  std::vector<std::string> suites = {"suite"};
  mettle::detail::file_uid_maker f;
  mettle::test_uid uid;

  logger.started_run();

  uid = f.make_file_uid();
  logger.started_suite(suites);
  logger.started_test({suites, "test 1", uid + 1});
  logger.passed_test({suites, "test 1", uid + 1}, log::test_output{}, 100ms);
  logger.started_test({suites, "test 2", uid + 2});
  logger.passed_test({suites, "test 2", uid + 2}, log::test_output{}, 100ms);

  suites.push_back("subsuite");
  logger.started_suite(suites);
  logger.started_test({suites, "test 3", uid + 3});
  logger.skipped_test({suites, "test 3", uid + 3}, "message\nmore");
  logger.failed_file({"test_file", uid}, "error\nmore");

  uid = f.make_file_uid();
  suites = {"second suite"};
  logger.started_suite(suites);
  logger.started_test({suites, "test 4", uid + 1});
  logger.passed_test({suites, "test 4", uid + 1}, output, 100ms);
  logger.ended_suite(suites);

  logger.ended_run();
}

inline void failing_test_and_file_run(mettle::log::file_logger &logger) {
  using namespace std::literals::chrono_literals;

  mettle::log::test_output output = {"standard output", "standard error"};
  std::vector<std::string> suites = {"suite"};
  mettle::detail::file_uid_maker f;
  mettle::test_uid uid;

  logger.started_run();

  uid = f.make_file_uid();
  logger.started_suite(suites);
  logger.started_test({suites, "test 1", uid + 1});
  logger.passed_test({suites, "test 1", uid + 1}, output, 100ms);
  logger.started_test({suites, "test 2", uid + 2});
  logger.failed_test({suites, "test 2", uid + 2}, "error", log::test_output{},
                     100ms);

  suites.push_back("subsuite");
  logger.started_suite(suites);
  logger.started_test({suites, "test 3", uid + 3});
  logger.skipped_test({suites, "test 3", uid + 3}, "message\nmore");
  logger.failed_file({"test_file", uid}, "error\nmore");

  uid = f.make_file_uid();
  suites = {"second suite"};
  logger.started_suite(suites);
  logger.started_test({suites, "test 4", uid + 1});
  logger.failed_test({suites, "test 4", uid + 1}, "error\nmore", output, 100ms);
  logger.ended_suite(suites);

  logger.ended_run();
}

#endif
