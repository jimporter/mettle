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

  _.test("METTLE_EXPECT(value, matcher)", []() {
    std::string message;
    int line = __LINE__ + 2; // The line the expectation is on.
    try {
      METTLE_EXPECT(false, equal_to(true));
    } catch(const expectation_error &e) {
      message = e.what();
    }

    std::ostringstream ss;
    ss << __FILE__ << ":" << line << "\nexpected: true\nactual:   false";
    expect(message, equal_to(ss.str()));
  });

  _.test("expect(value, matcher) with non-const", []() {
    expect(thing(), thing_value(1));
  });

  _.test("expect(desc, value, matcher) with non-const", []() {
    expect("description", thing(), thing_value(1));
  });

  _.test("METTLE_EXPECT(value, matcher)", []() {
    std::string message;
    int line = __LINE__ + 2; // The line the expectation is on.
    try {
      METTLE_EXPECT(false, equal_to(true));
    } catch(const expectation_error &e) {
      message = e.what();
    }

    std::ostringstream ss;
    ss << __FILE__ << ":" << line << "\nexpected: true\nactual:   false";
    expect(message, equal_to(ss.str()));
  });

  _.test("METTLE_EXPECT(desc, value, matcher)", []() {
    std::string message;
    int line = __LINE__ + 2; // The line the expectation is on.
    try {
      METTLE_EXPECT("description", false, equal_to(true));
    } catch(const expectation_error &e) {
      message = e.what();
    }

    std::ostringstream ss;
    ss << "description (" << __FILE__ << ":" << line
       << ")\nexpected: true\nactual:   false";
    expect(message, equal_to(ss.str()));
  });

  // This test ensures that we correctly resolve the call to mettle::expect,
  // since one of the overloads takes a string description as the first arg.
  _.test("METTLE_EXPECT(string, matcher)", []() {
    std::string message;
    int line = __LINE__ + 2; // The line the expectation is on.
    try {
      METTLE_EXPECT(std::string("foo"), equal_to(std::string("bar")));
    } catch(const expectation_error &e) {
      message = e.what();
    }

    std::ostringstream ss;
    ss << __FILE__ << ":" << line << "\nexpected: \"bar\"\nactual:   \"foo\"";
    expect(message, equal_to(ss.str()));
  });
});
