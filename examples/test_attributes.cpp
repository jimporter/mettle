#include <mettle.hpp>
using namespace mettle;

constexpr attribute slow("slow");

suite<> regular_suite("a regular suite", [](auto &_) {

  _.test("a skipped test", {skip}, []() {
    // ...
  });

  subsuite(_, "a skipped subsuite", {skip("broken")}, [](auto &_) {
    _.test("a test", []() {
      // ...
    });
  });

});

suite<> skipped_suite("a skipped suite", {skip}, [](auto &_) {

  _.test("a test", []() {
    // ...
  });

  subsuite(_, "a subsuite", [](auto &_) {
    _.test("a test", []() {
      // ...
    });
  });

});

suite<> slow_suite("a slow suite", {slow("takes too long")}, [](auto &_) {

  _.test("a test", {skip}, []() {
    // ...
  });

});
