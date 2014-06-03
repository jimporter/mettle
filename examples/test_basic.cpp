#include <mettle.hpp>
using namespace mettle;

suite<> basic("a basic suite", [](auto &_) {
  _.test("a test", []() {
    expect(true, equal_to(true));
  });

  _.test("another test", []() {
    expect(3, any_of(1, 2, 4));
  });

  _.skip_test("a skipped test", []() {
    expect(true, is_not(true));
  });

  for(int i = 0; i < 4; i++) {
    _.test("test number " + std::to_string(i), [i]() {
      expect(i % 2, less(2));
    });
  }
});
