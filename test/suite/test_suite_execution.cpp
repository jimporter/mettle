#include <mettle.hpp>
using namespace mettle;

#include "run_counter.hpp"

namespace mettle {
  std::string to_printable(const test_result &result) {
    std::ostringstream ss;
    ss << "test_result(" << to_printable(result.passed) << ", "
       << to_printable(result.message) << ")";
    return ss.str();
  }
}

template<typename T, typename U>
auto equal_test_result(T &&result, U &&message) {
  auto res = ensure_matcher(std::forward<T>(result));
  auto msg = ensure_matcher(std::forward<U>(message));

  std::ostringstream ss;
  ss << "test_result(" << res.desc() << ", " << msg.desc() << ")";

  return make_matcher(
    [res = std::move(res), msg = std::move(msg)]
    (const auto &actual) -> bool {
      return res(actual.passed) && msg(actual.message);
    }, ss.str()
  );
}

struct basic_fixture {
  basic_fixture() = default;
  basic_fixture(int data) : data(data) {}

  basic_fixture(const basic_fixture &) = delete;
  basic_fixture & operator =(const basic_fixture &) = delete;

  int data;
};

struct basic_factory {
  basic_factory(int data) : data(data) {}

  template<typename T>
  T make() {
    return { data };
  }

  int data = 0;
};

suite<> test_calling("test calling", [](auto &_) {

  _.test("passing test called", []() {
    run_counter<> test;
    auto s = make_suite<>("inner", [&test](auto &_){
      _.test("inner test", test);
    });

    auto result = s.tests()[0].function();
    expect(result, equal_test_result(true, ""));

    expect("test run count", test.runs(), equal_to(1));
  });

  _.test("failing test called", []() {
    run_counter<> test([]() {
      expect(false, equal_to(true));
    });
    auto s = make_suite<>("inner", [&test](auto &_){
      _.test("inner test", test);
    });

    auto result = s.tests()[0].function();
    expect(result, equal_test_result(
      false, regex_match("(.*\n)?expected: true\nactual:   false$")
    ));

    expect("test run count", test.runs(), equal_to(1));
  });

  _.test("setup and teardown called", []() {
    run_counter<> setup, teardown, test;
    auto s = make_suite<>("inner", [&setup, &teardown, &test](auto &_){
      _.setup(setup);
      _.teardown(teardown);
      _.test("inner test", test);
    });

    auto result = s.tests()[0].function();
    expect("test passed", result, equal_test_result(true, ""));

    expect("setup run count", setup.runs(), equal_to(1));
    expect("test run count", test.runs(), equal_to(1));
    expect("teardown run count", teardown.runs(), equal_to(1));
  });

  _.test("teardown called when test fails", []() {
    run_counter<> setup, teardown;
    run_counter<> test([]() {
      expect(false, equal_to(true));
    });
    auto s = make_suite<>("inner", [&setup, &teardown, &test](auto &_){
      _.setup(setup);
      _.teardown(teardown);
      _.test("inner test", test);
    });

    auto result = s.tests()[0].function();
    expect(result, equal_test_result(
      false, regex_match("(.*\n)?expected: true\nactual:   false$")
    ));

    expect("setup run count", setup.runs(), equal_to(1));
    expect("test run count", test.runs(), equal_to(1));
    expect("teardown run count", teardown.runs(), equal_to(1));
  });

  _.test("teardown not called when setup fails", []() {
    run_counter<> setup([]() {
      expect(false, equal_to(true));
    });
    run_counter<> teardown, test;
    auto s = make_suite<>("inner", [&setup, &teardown, &test](auto &_){
      _.setup(setup);
      _.teardown(teardown);
      _.test("inner test", test);
    });

    auto result = s.tests()[0].function();
    expect(result, equal_test_result(
      false, regex_match("(.*\n)?expected: true\nactual:   false$")
    ));

    expect("setup run count", setup.runs(), equal_to(1));
    expect("test run count", test.runs(), equal_to(0));
    expect("teardown run count", teardown.runs(), equal_to(0));
  });

  _.test("test fails when teardown fails", []() {
    run_counter<> teardown([]() {
      expect(false, equal_to(true));
    });
    run_counter<> setup, test;
    auto s = make_suite<>("inner", [&setup, &teardown, &test](auto &_){
      _.setup(setup);
      _.teardown(teardown);
      _.test("inner test", test);
    });

    auto result = s.tests()[0].function();
    expect(result, equal_test_result(
      false, regex_match("(.*\n)?expected: true\nactual:   false$")
    ));

    expect("setup run count", setup.runs(), equal_to(1));
    expect("test run count", test.runs(), equal_to(1));
    expect("teardown run count", teardown.runs(), equal_to(1));
  });

  _.test("test failure reported if test and teardown fail", []() {
    run_counter<> setup;
    run_counter<> teardown([]() {
      expect(std::string("teardown"), equal_to("foo"));
    });
    run_counter<> test([]() {
      expect(std::string("test"), equal_to("foo"));
    });
    auto s = make_suite<>("inner", [&setup, &teardown, &test](auto &_){
      _.setup(setup);
      _.teardown(teardown);
      _.test("inner test", test);
    });

    auto result = s.tests()[0].function();
    expect(result, equal_test_result(
      false, regex_match("(.*\n)?expected: \"foo\"\nactual:   \"test\"$")
    ));

    expect("setup run count", setup.runs(), equal_to(1));
    expect("test run count", test.runs(), equal_to(1));
    expect("teardown run count", teardown.runs(), equal_to(1));
  });

});

suite<basic_fixture> test_fixtures("suite fixtures", [](auto &_) {

  subsuite<int>(_, "subsuite", type_only, [](auto &_) {
    _.setup([](basic_fixture &f) {
      f.data++;
    });

    _.test("outer fixture was passed by reference", [](basic_fixture &f) {
      expect(f.data, equal_to(2));
    });

    subsuite<basic_fixture>(_, "sub-subsuite", basic_factory(5), [](auto &_) {
      _.setup([](basic_fixture &f, basic_fixture &) {
        f.data++;
      });

      _.test("outer fixture was passed by reference",
             [](basic_fixture &f, basic_fixture &) {
        expect(f.data, equal_to(3));
      });

      _.test("inner fixture was constructed by factory",
             [](basic_fixture &, basic_fixture &f2) {
        expect(f2.data, equal_to(5));
      });

    });

  });

  // Put the setup after the subsuite is created to ensure that order doesn't
  // matter.
  _.setup([](basic_fixture &f) {
    f.data = 1;
  });

});
