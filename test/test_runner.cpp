#include <mettle.hpp>
using namespace mettle;

struct my_test_logger : test_logger {
  my_test_logger() : tests_run(0), tests_passed(0), tests_skipped(0),
                     tests_failed(0) {}

  virtual void start_run() {}
  virtual void end_run() {}

  virtual void start_suite(const std::vector<std::string> &) {}
  virtual void end_suite(const std::vector<std::string> &) {}

  virtual void start_test(const test_name &) {
    tests_run++;
  }
  virtual void passed_test(const test_name &) {
    tests_passed++;
  }
  virtual void skipped_test(const test_name &) {
    tests_skipped++;
  }
  virtual void failed_test(const test_name &,
                           const std::string &) {
    tests_failed++;
  }

  size_t tests_run, tests_passed, tests_skipped, tests_failed;
};

suite<> test_runner("test runner", [](auto &_) {

  subsuite<>(_, "run_test()", [](auto &_) {
    _.test("passing test", []() {
      auto s = make_suite<>("inner", [](auto &_){
        _.test("test", []() {});
      });

      for(const auto &t : s) {
        auto result = detail::run_test(t.function);
        expect(result.passed, equal_to(true));
        expect(result.message, equal_to(""));
      }
    });

    _.test("failing test", []() {
      auto s = make_suite<>("inner", [](auto &_){
        _.test("test", []() {
          expect(true, equal_to(false));
        });
      });

      for(const auto &t : s) {
        auto result = detail::run_test(t.function);
        expect(result.passed, equal_to(false));
      }
    });

    _.test("aborting test", []() {
      auto s = make_suite<>("inner", [](auto &_){
        _.test("test", []() {
          abort();
        });
      });

      for(const auto &t : s) {
        auto result = detail::run_test(t.function);
        expect(result.passed, equal_to(false));
        expect(result.message, equal_to(strsignal(SIGABRT)));
      }
    });

    _.test("segfaulting test", []() {
      auto s = make_suite<>("inner", [](auto &_){
        _.test("test", []() {
          raise(SIGSEGV);
        });
      });

      for(const auto &t : s) {
        auto result = detail::run_test(t.function);
        expect(result.passed, equal_to(false));
        expect(result.message, equal_to(strsignal(SIGSEGV)));
      }
    });
  });

  subsuite<>(_, "run_tests()", [](auto &_) {
    _.test("suite of passing tests", []() {
      auto s = make_suites<>("inner", [](auto &_){
        _.test("test 1", []() {});
        _.test("test 2", []() {});
        _.test("test 3", []() {});
      });

      my_test_logger log;
      run_tests(s, log);
      expect(log.tests_run, equal_to(3));
      expect(log.tests_passed, equal_to(3));
      expect(log.tests_skipped, equal_to(0));
      expect(log.tests_failed, equal_to(0));
    });

    _.test("suite with failing tests", []() {
      auto s = make_suites<>("inner", [](auto &_){
        _.test("test 1", []() {
          expect(true, equal_to(false));
        });
        _.test("test 2", []() {});
        _.test("test 3", []() {});
      });

      my_test_logger log;
      run_tests(s, log);

      expect(log.tests_run, equal_to(3));
      expect(log.tests_passed, equal_to(2));
      expect(log.tests_skipped, equal_to(0));
      expect(log.tests_failed, equal_to(1));
    });


    _.test("suite with skipped tests", []() {
      auto s = make_suites<>("inner", [](auto &_){
        _.test("test 1", []() {});
        _.skip_test("test 2", []() {});
        _.test("test 3", []() {});
      });

      my_test_logger log;
      run_tests(s, log);

      expect(log.tests_run, equal_to(3));
      expect(log.tests_passed, equal_to(2));
      expect(log.tests_skipped, equal_to(1));
      expect(log.tests_failed, equal_to(0));
    });

    _.test("crashing tests don't crash framework", []() {
      auto s = make_suites<>("inner", [](auto &_){
        _.test("test 1", []() {});
        _.test("test 2", []() {
          abort();
        });
        _.test("test 3", []() {});
      });

      my_test_logger log;
      run_tests(s, log);

      expect(log.tests_run, equal_to(3));
      expect(log.tests_passed, equal_to(2));
      expect(log.tests_skipped, equal_to(0));
      expect(log.tests_failed, equal_to(1));
    });
  });
});
