#include <mettle.hpp>
using namespace mettle;

constexpr bool_attr slow("slow");
constexpr string_attr tag("tag");

suite<> regular_suite("a regular suite", [](auto &_) {

  _.test("a skipped test", {skip}, []() {
    // ...
  });

  _.test("a skipped test with a comment", {skip("broken")}, []() {
    // ...
  });

  _.test("a slow test", {slow}, []() {
    // ...
  });

  _.test("a tagged test", {tag("kitten")}, []() {
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

});
