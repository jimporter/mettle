#include <mettle/header_only.hpp>
using namespace mettle;

suite<> basic("a basic suite", [](auto &_) {

  _.test("a test", []() {
    expect(true, equal_to(true));
  });

  for(int i = 0; i < 4; i++) {
    _.test("test number " + std::to_string(i), [i]() {
      expect(i % 2, less(2));
    });
  }

  subsuite<>(_, "a subsuite", [](auto &_) {
    _.test("a sub-test", []() {
      expect(true, equal_to(true));
    });
  });

});
