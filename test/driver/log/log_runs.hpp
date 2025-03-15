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
  logger.started_test({suites, "test 1", uid + 1, "log_runs.cpp", 19});
  logger.passed_test({suites, "test 1", uid + 1, "log_runs.cpp", 19}, log::test_output{}, 100ms);
  logger.started_test({suites, "test 2", uid + 2, "log_runs.cpp", 21});
  logger.passed_test({suites, "test 2", uid + 2, "log_runs.cpp", 21}, log::test_output{}, 100ms);

  suites.push_back("subsuite");
  logger.started_suite(suites);
  logger.started_test({suites, "test 3", uid + 3, "log_runs.cpp", 26});
  logger.passed_test({suites, "test 3", uid + 3, "log_runs.cpp", 26}, output, 100ms);
  logger.ended_suite(suites);

  logger.ended_suite(suites);

  uid = f.make_file_uid();
  suites = {"second suite"};
  logger.started_suite(suites);
  logger.started_test({suites, "test 4", uid + 1, "log_runs.cpp", 35});
  logger.passed_test({suites, "test 4", uid + 1, "log_runs.cpp", 35}, output, 100ms);
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
  logger.started_test({suites, "test 1", uid + 1, "log_runs.cpp", 54});
  logger.passed_test({suites, "test 1", uid + 1, "log_runs.cpp", 54}, output, 100ms);
  logger.started_test({suites, "test 2", uid + 2, "log_runs.cpp", 56});
  logger.failed_test({suites, "test 2", uid + 2, "log_runs.cpp", 56}, "error", log::test_output{},
                     100ms);

  suites.push_back("subsuite");
  logger.started_suite(suites);
  logger.started_test({suites, "test 3", uid + 3, "log_runs.cpp", 62});
  logger.skipped_test({suites, "test 3", uid + 3, "log_runs.cpp", 62}, "message\nmore");
  logger.ended_suite(suites);

  logger.ended_suite(suites);

  uid = f.make_file_uid();
  suites = {"second suite"};
  logger.started_suite(suites);
  logger.started_test({suites, "test 4", uid + 1, "log_runs.cpp", 71});
  logger.failed_test({suites, "test 4", uid + 1, "log_runs.cpp", 71}, "error\nmore", output, 100ms);
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
  logger.started_test({suites, "test 1", uid + 1, "log_runs.cpp", 90});
  logger.passed_test({suites, "test 1", uid + 1, "log_runs.cpp", 90}, log::test_output{}, 100ms);
  logger.started_test({suites, "test 2", uid + 2, "log_runs.cpp", 92});
  logger.passed_test({suites, "test 2", uid + 2, "log_runs.cpp", 92}, log::test_output{}, 100ms);

  suites.push_back("subsuite");
  logger.started_suite(suites);
  logger.started_test({suites, "test 3", uid + 3, "log_runs.cpp", 97});
  logger.skipped_test({suites, "test 3", uid + 3, "log_runs.cpp", 97}, "message\nmore");
  logger.failed_file({"test_file", uid}, "error\nmore");

  uid = f.make_file_uid();
  suites = {"second suite"};
  logger.started_suite(suites);
  logger.started_test({suites, "test 4", uid + 1, "log_runs.cpp", 104});
  logger.passed_test({suites, "test 4", uid + 1, "log_runs.cpp", 104}, output, 100ms);
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
  logger.started_test({suites, "test 1", uid + 1, "log_runs.cpp", 123});
  logger.passed_test({suites, "test 1", uid + 1, "log_runs.cpp", 123}, output, 100ms);
  logger.started_test({suites, "test 2", uid + 2, "log_runs.cpp", 125});
  logger.failed_test({suites, "test 2", uid + 2, "log_runs.cpp", 125}, "error", log::test_output{},
                     100ms);

  suites.push_back("subsuite");
  logger.started_suite(suites);
  logger.started_test({suites, "test 3", uid + 3, "log_runs.cpp", 131});
  logger.skipped_test({suites, "test 3", uid + 3, "log_runs.cpp", 131}, "message\nmore");
  logger.failed_file({"test_file", uid}, "error\nmore");

  uid = f.make_file_uid();
  suites = {"second suite"};
  logger.started_suite(suites);
  logger.started_test({suites, "test 4", uid + 1, "log_runs.cpp", 138});
  logger.failed_test({suites, "test 4", uid + 1, "log_runs.cpp", 138}, "error\nmore", output,
                     100ms);
  logger.ended_suite(suites);

  logger.ended_run();
}

#endif
