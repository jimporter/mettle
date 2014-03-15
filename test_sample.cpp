#include "mettle.hpp"

using namespace mettle;

struct basic_data {
  int foo;
};

suite<basic_data> basic("basic suite", []() {
  basic.setup([](basic_data &) {
  });

  basic.teardown([](basic_data &) {
  });

  basic.test("a test", [](basic_data &) {
    expect(true, equals(true));
  });

  for(int i = 0; i < 4; i++) {
    basic.test("test number " + std::to_string(i), [i](basic_data &) {
      expect(i % 2, equals(0));
    });
  }
});

suite<> basic2("another basic suite", []() {
  basic2.test("a test", []() {
      expect(true, is_not(equals(true)));
  });
});
