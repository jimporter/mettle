#include <mettle.hpp>
using namespace mettle;

constexpr bool_attr slow("slow");
constexpr string_attr status("status");
constexpr list_attr tags("tags");

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

  _.test("a test with a status", {status("wip")}, []() {
    // ...
  });

  _.test("a tagged test", {tags("cat", "goat")}, []() {
    // ...
  });

  subsuite(_, "a skipped subsuite", {skip("broken")}, [](auto &_) {
    _.test("a test", []() {
      // ...
    });
  });

});

suite<> skipped_suite("a skipped suite", {skip}, [](auto &_) {

  _.test("a tagged test", {tags("goat")}, []() {
    // ...
  });

});
