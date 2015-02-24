#include <mettle.hpp>
using namespace mettle;

suite<> test_suite("suite", [](auto &_) {
  _.test("test", []() {
    expect(true, equal_to(false));
  });
});
