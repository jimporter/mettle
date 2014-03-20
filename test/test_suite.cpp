#include "mettle.hpp"
using namespace mettle;

suite<suites_list> test_suite("test suite", [](auto &_) {

  _.test("create a test suite", [](suites_list &suites) {
    suite<> inner("inner test suite", [](auto &_){
      _.test("inner test", []() {});
    }, suites);

    expect(suites, array(&inner));
  });

  _.test("create a test suite with fixture", [](suites_list &suites) {
    suite<int> inner("inner test suite", [](auto &_){
      _.test("inner test", [](int &) {});
    }, suites);

    expect(suites, array(&inner));
  });

});
