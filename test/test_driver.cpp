#include <mettle.hpp>
using namespace mettle;

suite<suites_list> test_driver("driver suite declaration", [](auto &_) {

  _.test("create a test suite", [](suites_list &suites) {
    suite<>("inner test suite", [](auto &_){
      _.test("inner test", []() {});
      _.skip_test("skipped test", []() {});
    }, suites);

    expect(suites.size(), equal_to<size_t>(1));

    auto &inner = suites[0];
    expect(inner.size(), equal_to<size_t>(2));
  });

  _.test("create a test suite with fixture", [](suites_list &suites) {
    suite<int>("inner test suite", [](auto &_){
      _.test("inner test", [](int &) {});
      _.skip_test("skipped test", [](int &) {});
    }, suites);

    expect(suites.size(), equal_to<size_t>(1));

    auto &inner = suites[0];
    expect(inner.size(), equal_to<size_t>(2));
  });

  _.test("create a test suite that throws", [](suites_list &suites) {
    try {
      suite<>("broken test suite", [](auto &){
        throw "bad";
      }, suites);
    } catch(...) {}

    expect(suites.size(), equal_to<size_t>(0));
  });

});
