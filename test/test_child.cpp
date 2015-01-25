#include <mettle.hpp>
using namespace mettle;

#include "helpers.hpp"
#include "../src/log_pipe.hpp"
#include "../src/test_file.hpp"
#include <mettle/driver/log/child.hpp>

struct recording_logger : log::file_logger {
  void started_run() {
    called = "started_run";
  }
  void ended_run() {
    called = "ended_run";
  }

  void started_file(const std::string &) {
    called = "started_file";
  }
  void ended_file(const std::string &) {
    called = "ended_file";
  }
  void failed_file(const std::string &, const std::string &) {
    called = "failed_file";
  }

  void started_suite(const std::vector<std::string> &actual_suites) {
    called = "started_suite";
    suites = actual_suites;
  }
  void ended_suite(const std::vector<std::string> &actual_suites) {
    called = "ended_suite";
    suites = actual_suites;
  }

  void started_test(const test_name &actual_test) {
    called = "started_test";
    test = actual_test;
  }
  void passed_test(const test_name &actual_test,
                   const log::test_output &actual_output,
                   log::test_duration actual_duration) {
    called = "passed_test";
    test = actual_test;
    output = actual_output;
    duration = actual_duration;
  }
  void failed_test(const test_name &actual_test,
                   const std::string &actual_message,
                   const log::test_output &actual_output,
                   log::test_duration actual_duration) {
    called = "failed_test";
    test = actual_test;
    message = actual_message;
    output = actual_output;
    duration = actual_duration;
  }
  void skipped_test(const test_name &actual_test,
                    const std::string &actual_message) {
    called = "skipped_test";
    test = actual_test;
    message = actual_message;
  }

  std::string called;
  std::vector<std::string> suites;
  test_name test;
  std::string message;
  log::test_output output;
  log::test_duration duration;
};

auto equal_test(const test_name &expected) {
  return make_matcher([expected](const test_name &actual) {
    // IDs probably won't match exactly, since after being piped, the 4 high
    // bytes are the file's UID. Thus, we only check the low 4 bytes.
    return actual.suites == expected.suites && actual.test == expected.test &&
           (actual.id & 0xffffffff) == (expected.id & 0xffffffff);
  }, to_printable(expected));
}

struct fixture {
  fixture() : pipe(parent), child(stream) {}

  recording_logger parent;
  log::pipe pipe;
  log::child child;
  std::stringstream stream;
};

suite<fixture> test_child("test child logger", [](auto &_) {

  _.test("started_run()", [](fixture &f) {
    f.child.started_run();
    f.pipe(f.stream);

    // Shouldn't be called, since we ignore started_run and ended_run.
    expect(f.parent.called, equal_to(""));
  });

  _.test("ended_run()", [](fixture &f) {
    f.child.ended_run();
    f.pipe(f.stream);

    // Shouldn't be called, since we ignore started_run and ended_run.
    expect(f.parent.called, equal_to(""));
  });

  _.test("started_suite()", [](fixture &f) {
    std::vector<std::string> suites = {"suite", "subsuite"};
    f.child.started_suite(suites);
    f.pipe(f.stream);

    expect(f.parent.called, equal_to("started_suite"));
    expect(f.parent.suites, equal_to(suites));
  });

  _.test("ended_suite()", [](fixture &f) {
    std::vector<std::string> suites = {"suite", "subsuite"};
    f.child.ended_suite(suites);
    f.pipe(f.stream);

    expect(f.parent.called, equal_to("ended_suite"));
    expect(f.parent.suites, equal_to(suites));
  });

  _.test("started_test()", [](fixture &f) {
    test_name test = {{"suite", "subsuite"}, "test", 1};
    f.child.started_test(test);
    f.pipe(f.stream);

    expect(f.parent.called, equal_to("started_test"));
    expect(f.parent.test, equal_test(test));
  });

  _.test("passed_test()", [](fixture &f) {
    test_name test = {{"suite", "subsuite"}, "test", 1};
    log::test_output output = {"stdout", "stderr"};
    log::test_duration duration(1000);

    f.child.passed_test(test, output, duration);
    f.pipe(f.stream);

    expect(f.parent.called, equal_to("passed_test"));
    expect(f.parent.test, equal_test(test));
    expect(f.parent.output.stdout_log, equal_to(output.stdout_log));
    expect(f.parent.output.stderr_log, equal_to(output.stderr_log));
    expect(f.parent.duration, equal_to(duration));
  });

  _.test("failed_test()", [](fixture &f) {
    test_name test = {{"suite", "subsuite"}, "test", 1};
    std::string message = "failure";
    log::test_output output = {"stdout", "stderr"};
    log::test_duration duration(1000);

    f.child.failed_test(test, message, output, duration);
    f.pipe(f.stream);

    expect(f.parent.called, equal_to("failed_test"));
    expect(f.parent.test, equal_test(test));
    expect(f.parent.message, equal_to(message));
    expect(f.parent.output.stdout_log, equal_to(output.stdout_log));
    expect(f.parent.output.stderr_log, equal_to(output.stderr_log));
    expect(f.parent.duration, equal_to(duration));
  });

  _.test("skipped_test()", [](fixture &f) {
    test_name test = {{"suite", "subsuite"}, "test", 1};
    std::string message = "message";
    f.child.skipped_test(test, message);
    f.pipe(f.stream);

    expect(f.parent.called, equal_to("skipped_test"));
    expect(f.parent.message, equal_to(message));
    expect(f.parent.test, equal_test(test));
  });

});

suite<> file_expansion("test file expansion", [](auto &_) {

  _.test("single word", []() {
    expect(test_file("filename").args(), array("filename"));
  });

  _.test("multiple words", []() {
    expect(test_file("file arg1 arg2").args(), array("file", "arg1", "arg2"));
    expect(test_file("file\targ1\targ2").args(), array("file", "arg1", "arg2"));
  });

  _.test("quoting", []() {
    expect(test_file("\"two words\"").args(), array("two words"));
    expect(test_file("'two words'").args(), array("two words"));
  });

  _.test("escaping", []() {
    expect(test_file("arg\\ 1 arg\\ 2").args(), array("arg 1", "arg 2"));
    expect(test_file("o\\'toole").args(), array("o'toole"));
    expect(test_file("\\\"quote\\\"").args(), array("\"quote\""));
  });

  _.test("invalid syntax", []() {
    expect([]() { test_file("glob["); },
           thrown<std::runtime_error>("invalid glob \"glob[\""));
  });

});
