#include <mettle.hpp>
using namespace mettle;

suite<> sub("suite with subsuites", [](auto &_) {

  _.test("a test", []() {
    // ...
  });

  _.template subsuite<>("subsuite", [](auto &_) {
    _.test("a subtest", []() {
      // ...
    });
  });

  subsuite<>(_, "another subsuite", [](auto &_) {
    _.test("a subtest", []() {
      // ...
    });
  });

});

