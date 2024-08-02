#include <mettle.hpp>
using namespace mettle;

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
  _.test("expect(value, matcher)", []() {
    std::string message;
    try {
      expect(false, equal_to(true));
    } catch(const expectation_error &e) {
      message = e.what();
    }

    std::ostringstream ss;
#ifndef METTLE_NO_SOURCE_LOCATION
    int line = __LINE__ - 7; // The line the expectation is on.
    ss << __FILE__ << ":" << line << "\n";
#endif
    ss << "expected: true\nactual:   false";
    expect(message, equal_to(ss.str()));
  });

  _.test("expect(desc, value, matcher)", []() {
    std::string message;
    try {
      expect("description", false, equal_to(true));
    } catch(const expectation_error &e) {
      message = e.what();
    }

    std::ostringstream ss;
    ss << "description";
#ifndef METTLE_NO_SOURCE_LOCATION
    int line = __LINE__ - 8; // The line the expectation is on.
    ss << " (" << __FILE__ << ":" << line << ")";
#endif
    ss << "\nexpected: true\nactual:   false";
    expect(message, equal_to(ss.str()));
  });
});
