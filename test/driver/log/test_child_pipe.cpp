#include <mettle.hpp>
using namespace mettle;

#include "../../helpers.hpp"
#include "../../../src/mettle/log_pipe.hpp"
#include <mettle/driver/log/child.hpp>

struct recording_logger : log::file_logger {
  void started_run() override {
    called = "started_run";
  }
  void ended_run() override {
    called = "ended_run";
  }

  void started_file(const test_file &) override {
    called = "started_file";
  }
  void ended_file(const test_file &) override {
    called = "ended_file";
  }
  void failed_file(const test_file &, const std::string &) override {
    called = "failed_file";
  }

  void started_suite(const std::vector<std::string> &actual_suites) override {
    called = "started_suite";
    suites = actual_suites;
  }
  void ended_suite(const std::vector<std::string> &actual_suites) override {
    called = "ended_suite";
    suites = actual_suites;
  }

  void started_test(const test_name &actual_test) override {
    called = "started_test";
    test = actual_test;
  }
  void passed_test(const test_name &actual_test,
                   const log::test_output &actual_output,
                   log::test_duration actual_duration) override {
    called = "passed_test";
    test = actual_test;
    output = actual_output;
    duration = actual_duration;
  }
  void failed_test(const test_name &actual_test,
                   const std::string &actual_message,
                   const log::test_output &actual_output,
                   log::test_duration actual_duration) override {
    called = "failed_test";
    test = actual_test;
    message = actual_message;
    output = actual_output;
    duration = actual_duration;
  }
  void skipped_test(const test_name &actual_test,
                    const std::string &actual_message) override {
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

auto equal_test_name(const test_name &expected) {
  return basic_matcher(
    expected,
    [](const test_name &actual, const test_name &expected) {
      // The actual ID is the local (expected) ID plus the file ID, which is
      // always store in the 4 high bytes. For this test, we assume it's
      // 1 << 32.
      return actual.suites == expected.suites && actual.name == expected.name &&
             (actual.id == expected.id + (test_uid(1) << 32));
    }, ""
  );
}

struct fixture {
  fixture() : pipe(parent, test_uid(1) << 32), child(stream) {}

  recording_logger parent;
  log::pipe pipe;
  log::child child;
  std::stringstream stream;
};

suite<fixture> test_child("child/pipe loggers", [](auto &_) {

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
    expect(f.parent.test, equal_test_name(test));
  });

  _.test("passed_test()", [](fixture &f) {
    test_name test = {{"suite", "subsuite"}, "test", 1};
    log::test_output output = {"stdout", "stderr"};
    log::test_duration duration(1000);

    f.child.passed_test(test, output, duration);
    f.pipe(f.stream);

    expect(f.parent.called, equal_to("passed_test"));
    expect(f.parent.test, equal_test_name(test));
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
    expect(f.parent.test, equal_test_name(test));
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
    expect(f.parent.test, equal_test_name(test));
  });

});
