#include <mettle.hpp>
using namespace mettle;

suite<suites_list> test_global_suite("global suite declaration", [](auto &_) {

  _.test("create a test suite", [](suites_list &suites) {
    suite<>(suites, "inner test suite", [](auto &_){
      _.test("inner test", []() {});
      _.test("skipped test", {skip}, []() {});
    });

    expect(suites.size(), equal_to(1));

    auto &inner = suites[0];
    expect(inner.tests().size(), equal_to(2));
  });

  _.test("create a test suite with fixture", [](suites_list &suites) {
    suite<int>(suites, "inner test suite", [](auto &_){
      _.test("inner test", [](int &) {});
      _.test("skipped test", {skip}, [](int &) {});
    });

    expect(suites.size(), equal_to(1));

    auto &inner = suites[0];
    expect(inner.tests().size(), equal_to(2));
  });

  _.test("create a test suite with parameterized fixtures",
         [](suites_list &suites) {
    suite<int, float>(suites, "inner test suite", [](auto &_){
      using Fixture = fixture_type_t<decltype(_)>;
      (void)Fixture{};

      _.test("inner test", [](auto &) {});
      _.test("skipped test", {skip}, [](auto &) {});
    });

    expect(suites.size(), equal_to(2));

    auto &int_suite = suites[0];
    expect(int_suite.tests().size(), equal_to(2));
    auto &float_suite = suites[1];
    expect(float_suite.tests().size(), equal_to(2));
  });

  _.test("create a skipped test suite", [](suites_list &suites) {
    suite<>(suites, "inner test suite", {skip}, [](auto &_){
      _.test("inner test", []() {});
      _.test("skipped test", {skip}, []() {});
    });

    expect(suites.size(), equal_to(1));

    auto &inner = suites[0];
    expect(inner.tests().size(), equal_to(2));
  });

  _.test("create a skipped test suite with fixture", [](suites_list &suites) {
    suite<int>(suites, "inner test suite", {skip}, [](auto &_){
      _.test("inner test", [](int &) {});
      _.test("skipped test", {skip}, [](int &) {});
    });

    expect(suites.size(), equal_to(1));

    auto &inner = suites[0];
    expect(inner.tests().size(), equal_to(2));
  });

  _.test("create a skipped test suite with parameterized fixtures",
         [](suites_list &suites) {
    suite<int, float>(suites, "inner test suite", {skip}, [](auto &_){
      using Fixture = fixture_type_t<decltype(_)>;
      (void)Fixture{};

      _.test("inner test", [](auto &) {});
      _.test("skipped test", {skip}, [](auto &) {});
    });

    expect(suites.size(), equal_to(2));

    auto &int_suite = suites[0];
    expect(int_suite.tests().size(), equal_to(2));
    auto &float_suite = suites[1];
    expect(float_suite.tests().size(), equal_to(2));
  });

  _.test("create a test suite that throws", [](suites_list &suites) {
    try {
      suite<>(suites, "broken test suite", [](auto &){
        throw "bad";
      });
    } catch(...) {}

    expect(suites.size(), equal_to(0));
  });

});
