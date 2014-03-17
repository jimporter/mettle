#include "mettle.hpp"

using namespace mettle;

struct basic_data {
  int foo;
};

suite<basic_data> basic("basic suite", [](suite<basic_data>& _) {
  _.setup([](basic_data &) {
  });

  _.teardown([](basic_data &) {
  });

  _.test("a test", [](basic_data &) {
    expect(true, equals(true));
  });

  for(int i = 0; i < 4; i++) {
    _.test("test number " + std::to_string(i), [i](basic_data &) {
      expect(i % 2, equals(0));
    });
  }
});

suite<> basic2("another basic suite", [](suite<>& _) {
  _.test("a test", []() {
    expect(true, is_not(equals(true)));
  });

  _.skip_test("a skipped test", []() {
  });
});
