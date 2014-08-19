#include <mettle.hpp>
using namespace mettle;

suite<suites_list> test_driver("driver suite declaration", [](auto &_) {

  _.test("create a test suite", [](suites_list &suites) {
    suite<>(suites, "inner test suite", [](auto &_){
      _.test("inner test", []() {});
      _.skip_test("skipped test", []() {});
    });

    expect(suites.size(), equal_to<size_t>(1));

    auto &inner = suites[0];
    expect(inner.size(), equal_to<size_t>(2));
  });

  _.test("create a test suite with fixture", [](suites_list &suites) {
    suite<int>(suites, "inner test suite", [](auto &_){
      _.test("inner test", [](int &) {});
      _.skip_test("skipped test", [](int &) {});
    });

    expect(suites.size(), equal_to<size_t>(1));

    auto &inner = suites[0];
    expect(inner.size(), equal_to<size_t>(2));
  });

  _.test("create a test suite with parameterized fixtures",
         [](suites_list &suites) {
    suite<int, float>(suites, "inner test suite", [](auto &_){
      using Fixture = fixture_type_t<decltype(_)>;

      _.test("inner test", [](auto &) {});
      _.skip_test("skipped test", [](auto &) {});
    });

    expect(suites.size(), equal_to<size_t>(2));

    auto &int_suite = suites[0];
    expect(int_suite.size(), equal_to<size_t>(2));
    auto &float_suite = suites[1];
    expect(float_suite.size(), equal_to<size_t>(2));
  });

  _.test("create a skipped test suite", [](suites_list &suites) {
    skip_suite<>(suites, "inner test suite", [](auto &_){
      _.test("inner test", []() {});
      _.skip_test("skipped test", []() {});
    });

    expect(suites.size(), equal_to<size_t>(1));

    auto &inner = suites[0];
    expect(inner.size(), equal_to<size_t>(2));
  });

  _.test("create a skipped test suite with fixture", [](suites_list &suites) {
    skip_suite<int>(suites, "inner test suite", [](auto &_){
      _.test("inner test", [](int &) {});
      _.skip_test("skipped test", [](int &) {});
    });

    expect(suites.size(), equal_to<size_t>(1));

    auto &inner = suites[0];
    expect(inner.size(), equal_to<size_t>(2));
  });

  _.test("create a skipped test suite with parameterized fixtures",
         [](suites_list &suites) {
    skip_suite<int, float>(suites, "inner test suite", [](auto &_){
      using Fixture = fixture_type_t<decltype(_)>;

      _.test("inner test", [](auto &) {});
      _.skip_test("skipped test", [](auto &) {});
    });

    expect(suites.size(), equal_to<size_t>(2));

    auto &int_suite = suites[0];
    expect(int_suite.size(), equal_to<size_t>(2));
    auto &float_suite = suites[1];
    expect(float_suite.size(), equal_to<size_t>(2));
  });

  _.test("create a test suite that throws", [](suites_list &suites) {
    try {
      suite<>(suites, "broken test suite", [](auto &){
        throw "bad";
      });
    } catch(...) {}

    expect(suites.size(), equal_to<size_t>(0));
  });

});
