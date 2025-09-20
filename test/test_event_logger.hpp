#ifndef INC_METTLE_TEST_TEST_EVENT_LOGGER_HPP
#define INC_METTLE_TEST_TEST_EVENT_LOGGER_HPP

#include <set>
#include <string>
#include <vector>

#include <mettle/driver/log/core.hpp>

struct test_event_logger : log::file_logger {
  test_event_logger() {}

  void started_run() override {
    events.push_back("started_run");
  }
  void ended_run() override {
    events.push_back("ended_run");
  }

  void started_suite(const std::vector<suite_name> &) override {
    events.push_back("started_suite");
  }
  void ended_suite(const std::vector<suite_name> &) override {
    events.push_back("ended_suite");
  }

  void started_test(const test_name &test) override {
    events.push_back("started_test");
    tests.insert(test);
  }
  void passed_test(const test_name &test, const log::test_output &,
                   log::test_duration) override {
    events.push_back("passed_test");
    tests.insert(test);
  }
  void failed_test(const test_name &test, const std::string &,
                   const log::test_output &, log::test_duration) override {
    events.push_back("failed_test");
    tests.insert(test);
  }
  void skipped_test(const test_name &test, const std::string &) override {
    events.push_back("skipped_test");
    tests.insert(test);
  }

  void started_file(const test_file &file) override {
    events.push_back("started_file");
    files.insert(file);
  }
  void ended_file(const test_file &file) override {
    events.push_back("ended_file");
    files.insert(file);
  }
  void failed_file(const test_file &file, const std::string &) override {
    events.push_back("failed_file");
    files.insert(file);
  }

  std::vector<std::string> events;
  std::set<test_file> files;
  std::set<test_name> tests;
};

#endif
