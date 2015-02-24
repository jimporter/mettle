#ifndef INC_METTLE_TEST_TEST_EVENT_LOGGER_HPP
#define INC_METTLE_TEST_TEST_EVENT_LOGGER_HPP

#include <string>
#include <vector>

#include <mettle/driver/log/core.hpp>

struct test_event_logger : log::file_logger {
  test_event_logger() {}

  void started_run() {
    events.push_back("started_run");
  }
  void ended_run() {
    events.push_back("ended_run");
  }

  void started_suite(const std::vector<std::string> &) {
    events.push_back("started_suite");
  }
  void ended_suite(const std::vector<std::string> &) {
    events.push_back("ended_suite");
  }

  void started_test(const test_name &) {
    events.push_back("started_test");
  }
  void passed_test(const test_name &, const log::test_output &,
                   log::test_duration) {
    events.push_back("passed_test");
  }
  void failed_test(const test_name &, const std::string &,
                   const log::test_output &, log::test_duration) {
    events.push_back("failed_test");
  }
  void skipped_test(const test_name &, const std::string &) {
    events.push_back("skipped_test");
  }

  void started_file(const std::string &) {
    events.push_back("started_file");
  }
  void ended_file(const std::string &) {
    events.push_back("ended_file");
  }
  void failed_file(const std::string &, const std::string &) {
    events.push_back("failed_file");
  }

  std::vector<std::string> events;
};

#endif
