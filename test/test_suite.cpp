#include "mettle.hpp"
using namespace mettle;

inline auto match_test(const std::string &name, bool skip) {
  std::stringstream s;
  if(skip)
    s << "skipped ";
  s << "test named \"" << name << "\"";
  return make_matcher([name, skip](const suite_base::test_info &actual) {
    return actual.name == name && actual.skip == skip;
  }, s.str());
}

suite<suites_list> test_suite("suite creation", [](auto &_) {

  _.test("create a test suite", [](suites_list &suites) {
    suite<> inner("inner test suite", [](auto &_){
      _.test("inner test", []() {});
      _.skip_test("skipped test", []() {});
    }, suites);

    expect(suites, array(&inner));
    expect(inner.size(), equal_to<size_t>(2));
    expect(inner, array(
      match_test("inner test", false), match_test("skipped test", true)
    ));
  });

  _.test("create a test suite with fixture", [](suites_list &suites) {
    suite<int> inner("inner test suite", [](auto &_){
      _.test("inner test", [](int &) {});
      _.skip_test("skipped test", [](int &) {});
    }, suites);

    expect(suites, array(&inner));
    expect(inner.size(), equal_to<size_t>(2));
    expect(inner, array(
      match_test("inner test", false), match_test("skipped test", true)
    ));
  });

  _.test("create a test suite with setup/teardown", [](suites_list &suites) {
    suite<> inner("inner test suite", [](auto &_){
      _.setup([]() {});
      _.teardown([]() {});
      _.test("inner test", []() {});
      _.skip_test("skipped test", []() {});
    }, suites);

    expect(suites, array(&inner));
    expect(inner.size(), equal_to<size_t>(2));
    expect(inner, array(
      match_test("inner test", false), match_test("skipped test", true)
    ));
  });

  _.test("create a test suite with fixture and setup/teardown",
         [](suites_list &suites) {
    suite<int> inner("inner test suite", [](auto &_){
      _.setup([](int &) {});
      _.teardown([](int &) {});
      _.test("inner test", [](int &) {});
      _.skip_test("skipped test", [](int &) {});
    }, suites);

    expect(suites, array(&inner));
    expect(inner.size(), equal_to<size_t>(2));
    expect(inner, array(
      match_test("inner test", false), match_test("skipped test", true)
    ));
  });

  _.test("create a test suite that throws", [](suites_list &suites) {
    try {
      suite<int> inner("broken test suite", [](auto &){
        throw "bad";
      }, suites);
    } catch(...) {}

    expect(suites, array());
  });

});
