#include <mettle.hpp>
using namespace mettle;

#include "../src/file_runner.hpp"
#include "../src/log_pipe.hpp"
#include "../src/libmettle/log_child.hpp"

struct recording_logger : log::test_logger {
  void started_run() {
    called = "started_run";
  }
  void ended_run() {
    called = "ended_run";
  }

  void started_suite(const std::vector<std::string> &actual_suites) {
    called = "started_suite";
    suites = actual_suites;
  }
  void ended_suite(const std::vector<std::string> &actual_suites) {
    called = "ended_suite";
    suites = actual_suites;
  }

  void started_test(const log::test_name &actual_test) {
    called = "started_test";
    test = actual_test;
  }
  void passed_test(const log::test_name &actual_test,
                   const log::test_output &actual_output) {
    called = "passed_test";
    test = actual_test;
    output = actual_output;
  }
  void failed_test(const log::test_name &actual_test,
                   const std::string &actual_message,
                   const log::test_output &actual_output) {
    called = "failed_test";
    test = actual_test;
    message = actual_message;
    output = actual_output;
  }
  void skipped_test(const log::test_name &actual_test,
                    const std::string &actual_message) {
    called = "skipped_test";
    test = actual_test;
    message = actual_message;
  }

  void failed_file(const std::string &, const std::string &) {
    called = "failed_file";
  }

  std::string called;
  std::vector<std::string> suites;
  log::test_name test;
  std::string message;
  log::test_output output;
};

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
    log::test_name test = {{"suite", "subsuite"}, "test", 1};
    f.child.started_test(test);
    f.pipe(f.stream);

    expect(f.parent.called, equal_to("started_test"));
    expect(f.parent.test, equal_to(test));
  });

  _.test("passed_test()", [](fixture &f) {
    log::test_name test = {{"suite", "subsuite"}, "test", 1};
    log::test_output output = {"stdout", "stderr"};

    f.child.passed_test(test, output);
    f.pipe(f.stream);

    expect(f.parent.called, equal_to("passed_test"));
    expect(f.parent.test, equal_to(test));
    expect(f.parent.output.stdout, equal_to(output.stdout));
    expect(f.parent.output.stderr, equal_to(output.stderr));
  });

  _.test("failed_test()", [](fixture &f) {
    log::test_name test = {{"suite", "subsuite"}, "test", 1};
    std::string message = "failure";
    log::test_output output = {"stdout", "stderr"};

    f.child.failed_test(test, message, output);
    f.pipe(f.stream);

    expect(f.parent.called, equal_to("failed_test"));
    expect(f.parent.test, equal_to(test));
    expect(f.parent.message, equal_to(message));
    expect(f.parent.output.stdout, equal_to(output.stdout));
    expect(f.parent.output.stderr, equal_to(output.stderr));
  });

  _.test("skipped_test()", [](fixture &f) {
    log::test_name test = {{"suite", "subsuite"}, "test", 1};
    std::string message = "message";
    f.child.skipped_test(test, message);
    f.pipe(f.stream);

    expect(f.parent.called, equal_to("skipped_test"));
    expect(f.parent.message, equal_to(message));
    expect(f.parent.test, equal_to(test));
  });

});
