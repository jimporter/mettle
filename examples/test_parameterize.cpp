#include <mettle.hpp>
using namespace mettle;

suite<int, float> param("parameterized suite", [](auto &_) {

  _.test("a test", [](auto &fixture) {
    expect(fixture, equal_to(0));
  });

  subsuite<bool>(_, "a subsuite", [](auto &_) {
    _.test("another test", [](auto &fixture1, bool &fixture2) {
      expect(fixture1 && fixture2, equal_to(false));
    });
  });

});

suite<> fix("fixture suite", [](auto &_) {

  subsuite<int, float>(_, "blah", [](auto &_) {
    _.test("my test", [](auto &) {
    });
  });

});
