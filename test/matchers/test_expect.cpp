#include <mettle.hpp>
using namespace mettle;

#include <mettle/driver/log/format.hpp>

struct thing {
  int value() {
    return 1;
  }
};

auto thing_value(int value) {
  return basic_matcher(
    ensure_matcher(value),
    [](thing &actual, const auto &expected) -> bool {
      return expected(actual.value());
    }, "value() "
  );
}


suite<> test_expect("expect()", [](auto &_) {
  using namespace std::string_literals;

  _.test("expect(value, matcher)", []() {
    std::optional<expectation_error> err;
    try {
      expect(false, equal_to(true));
    } catch(const expectation_error &e) {
      err = e;
    }

#ifndef METTLE_NO_SOURCE_LOCATION
    int line = __LINE__ - 6; // The line the expectation is on.
    expect(err->location().file_name(), equal_to(__FILE__));
    expect(err->location().line(), equal_to(line));
#endif
    expect(err->desc(), equal_to(""));
    expect(err->what(), equal_to("expected: true\nactual:   false"s));
  });

  _.test("expect(desc, value, matcher)", []() {
    std::optional<expectation_error> err;
    try {
      expect("description", false, equal_to(true));
    } catch(const expectation_error &e) {
      err = e;
    }

#ifndef METTLE_NO_SOURCE_LOCATION
    int line = __LINE__ - 6; // The line the expectation is on.
    expect(err->location().file_name(), equal_to(__FILE__));
    expect(err->location().line(), equal_to(line));
#endif
    expect(err->desc(), equal_to("description"));
    expect(err->what(), equal_to("expected: true\nactual:   false"s));
  });
});
