#include "mettle.hpp"

using namespace mettle;

struct basic_data {
  int foo;
};

suite<basic_data> basic("basic suite", [](auto &_) {
  _.setup([](basic_data &) {
  });

  _.teardown([](basic_data &) {
  });

  _.test("a test", [](basic_data &) {
    expect(true, equal_to(true));
  });

  for(int i = 0; i < 4; i++) {
    _.test("test number " + std::to_string(i), [i](basic_data &) {
      expect(i % 2, equal_to(0));
    });
  }
});

suite<> basic2("another basic suite", [](auto &_) {
  _.test("a test", []() {
    expect(true, is_not(true));
  });

  _.test("another test", []() {
    expect(3, any_of(1, 2, 4));
  });

  _.skip_test("a skipped test", []() {
  });
});
