#include <mettle.hpp>
using namespace mettle;

#include <string>

struct my_fixture {
  int value;
};

suite<my_fixture> fix("fixture suite", [](auto &_) {

  _.setup([](my_fixture &f) {
    f.value = 0;
  });

  _.test("a test", [](my_fixture &f) {
    expect(f.value, equal_to(0));
  });

  subsuite<std::string>(_, "subsuite", [](auto &_) {
    _.setup([](my_fixture &f, std::string &s) {
      f.value++;
      s = "boop";
    });

    _.test("a subtest", [](my_fixture &f, std::string &s) {
      expect(f.value, equal_to(1));
      expect(s, equal_to("boop"));
    });
  });

  subsuite<float, float>(_, "another subsuite", [](auto &_) {
    _.setup([](my_fixture &f, float &x, float &y) {
      x = 1;
      y = 2;
    });

    _.test("a subtest", [](my_fixture &f, float &x, float &y) {
      expect(y - x, equal_to(1));
    });
  });

});

