#ifndef INC_METTLE_TEST_DRIVER_LOG_LOG_RUNS_HPP
#define INC_METTLE_TEST_DRIVER_LOG_LOG_RUNS_HPP

#include <mettle/test_uid.hpp>
#include <mettle/driver/log/core.hpp>

inline void passing_run(mettle::log::test_logger &logger) {
  using namespace std::literals::chrono_literals;

  mettle::log::test_output output = {"standard output", "standard error"};
  std::vector<suite_name> suites = {{"suite", "file.cpp", 1}};
  mettle::detail::file_uid_maker f;
  mettle::test_uid uid;

  logger.started_run();

  uid = f.make_file_uid();
  logger.started_suite(suites);
  logger.started_test({uid + 1, suites, "test 1", "file.cpp", 10});
  logger.passed_test({uid + 1, suites, "test 1", "file.cpp", 10},
                     log::test_output{}, 100ms);
  logger.started_test({uid + 2, suites, "test 2", "file.cpp", 20});
  logger.passed_test({uid + 2, suites, "test 2", "file.cpp", 20},
                     log::test_output{}, 100ms);

  suites.push_back({"subsuite", "file.cpp", 3});
  logger.started_suite(suites);
  logger.started_test({uid + 3, suites, "test 3", "file.cpp", 30});
  logger.passed_test({uid + 3, suites, "test 3", "file.cpp", 30},
                     output, 100ms);
  logger.ended_suite(suites);

  logger.ended_suite(suites);

  uid = f.make_file_uid();
  suites = {{"second suite", "file.cpp", 4}};
  logger.started_suite(suites);
  logger.started_test({uid + 1, suites, "test 4", "file.cpp", 40});
  logger.passed_test({uid + 1, suites, "test 4", "file.cpp", 40},
                     output, 100ms);
  logger.ended_suite(suites);

  logger.ended_run();
}

inline void failing_run(mettle::log::test_logger &logger) {
  using namespace std::literals::chrono_literals;

  mettle::log::test_output output = {"standard output", "standard error"};
  std::vector<suite_name> suites = {{"suite", "file.cpp", 1}};
  mettle::detail::file_uid_maker f;
  mettle::test_uid uid;

  logger.started_run();

  uid = f.make_file_uid();
  logger.started_suite(suites);
  logger.started_test({uid + 1, suites, "test 1", "file.cpp", 10});
  logger.passed_test({uid + 1, suites, "test 1", "file.cpp", 10},
                     output, 100ms);
  logger.started_test({uid + 2, suites, "test 2", "file.cpp", 20});
  logger.failed_test({uid + 2, suites, "test 2", "file.cpp", 20},
                     {"desc", "error", "file.cpp", 22}, {}, 100ms);

  suites.push_back({"subsuite", "file.cpp", 3});
  logger.started_suite(suites);
  logger.started_test({uid + 3, suites, "test 3", "file.cpp", 30});
  logger.skipped_test({uid + 3, suites, "test 3", "file.cpp", 30},
                      "message\nmore");
  logger.ended_suite(suites);

  logger.ended_suite(suites);

  uid = f.make_file_uid();
  suites = {{"second suite", "file.cpp", 4}};
  logger.started_suite(suites);
  logger.started_test({uid + 1, suites, "test 4", "file.cpp", 40});
  logger.failed_test({uid + 1, suites, "test 4", "file.cpp", 40},
                     {"desc", "error\nmore", "file.cpp", 44}, output, 100ms);
  logger.ended_suite(suites);

  logger.ended_run();
}

inline void failing_file_run(mettle::log::file_logger &logger) {
  using namespace std::literals::chrono_literals;

  mettle::log::test_output output = {"standard output", "standard error"};
  std::vector<suite_name> suites = {{"suite", "file.cpp", 1}};
  mettle::detail::file_uid_maker f;
  mettle::test_uid uid;

  logger.started_run();

  uid = f.make_file_uid();
  logger.started_suite(suites);
  logger.started_test({uid + 1, suites, "test 1", "file.cpp", 10});
  logger.passed_test({uid + 1, suites, "test 1", "file.cpp", 10},
                     log::test_output{}, 100ms);
  logger.started_test({uid + 2, suites, "test 2", "file.cpp", 20});
  logger.passed_test({uid + 2, suites, "test 2", "file.cpp", 20},
                     log::test_output{}, 100ms);

  suites.push_back({"subsuite", "file.cpp", 3});
  logger.started_suite(suites);
  logger.started_test({uid + 3, suites, "test 3", "file.cpp", 30});
  logger.skipped_test({uid + 3, suites, "test 3", "file.cpp", 30},
                      "message\nmore");
  logger.failed_file({uid, "file.cpp"}, "error\nmore");

  uid = f.make_file_uid();
  suites = {{"second suite", "file.cpp", 4}};
  logger.started_suite(suites);
  logger.started_test({uid + 1, suites, "test 4", "file.cpp", 40});
  logger.passed_test({uid + 1, suites, "test 4", "file.cpp", 40},
                     output, 100ms);
  logger.ended_suite(suites);

  logger.ended_run();
}

inline void failing_test_and_file_run(mettle::log::file_logger &logger) {
  using namespace std::literals::chrono_literals;

  mettle::log::test_output output = {"standard output", "standard error"};
  std::vector<suite_name> suites = {{"suite", "file.cpp", 1}};
  mettle::detail::file_uid_maker f;
  mettle::test_uid uid;

  logger.started_run();

  uid = f.make_file_uid();
  logger.started_suite(suites);
  logger.started_test({uid + 1, suites, "test 1", "file.cpp", 10});
  logger.passed_test({uid + 1, suites, "test 1", "file.cpp", 10},
                     output, 100ms);
  logger.started_test({uid + 2, suites, "test 2", "file.cpp", 20});
  logger.failed_test({uid + 2, suites, "test 2", "file.cpp", 20},
                     {"desc", "error", "file.cpp", 22}, {}, 100ms);

  suites.push_back({"subsuite", "file.cpp", 3});
  logger.started_suite(suites);
  logger.started_test({uid + 3, suites, "test 3", "file.cpp", 30});
  logger.skipped_test({uid + 3, suites, "test 3", "file.cpp", 30},
                      "message\nmore");
  logger.failed_file({uid, "file.cpp"}, "error\nmore");

  uid = f.make_file_uid();
  suites = {{"second suite", "file.cpp", 4}};
  logger.started_suite(suites);
  logger.started_test({uid + 1, suites, "test 4", "file.cpp", 40});
  logger.failed_test({uid + 1, suites, "test 4", "file.cpp", 40},
                     {"desc", "error\nmore", "file.cpp", 44}, output, 100ms);
  logger.ended_suite(suites);

  logger.ended_run();
}

#endif
