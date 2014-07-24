#include <mettle.hpp>
using namespace mettle;

suite<> regular("a regular suite", [](auto &_) {

  _.skip_test("a skipped test", []() {
    // ...
  });

  skip_subsuite(_, "a skipped subsuite", [](auto &_) {
    _.test("a test", []() {
      // ...
    });
  });

});

skip_suite<> skip("a skipped suite", [](auto &_) {

  _.test("a test", []() {
    // ...
  });

  subsuite(_, "a subsuite", [](auto &_) {
    _.test("a test", []() {
      // ...
    });
  });

});
