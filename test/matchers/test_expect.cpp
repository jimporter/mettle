#include <mettle.hpp>
using namespace mettle;

suite<> test_expect("expect()", [](auto &_) {
  _.test("expect(value, matcher)", []() {
    std::string message;
    try {
      expect(false, equal_to(true));
    }
    catch(const expectation_error &e) {
      message = e.what();
    }
    expect(message, equal_to("expected: true\nactual:   false"));
  });

  _.test("expect(desc, value, matcher)", []() {
    std::string message;
    try {
      expect("description", false, equal_to(true));
    }
    catch(const expectation_error &e) {
      message = e.what();
    }
    expect(message, equal_to("description\nexpected: true\nactual:   false"));
  });

  _.test("METTLE_EXPECT(value, matcher)", []() {
    std::string message;
    int line = __LINE__ + 2; // The line the expectation is on.
    try {
      METTLE_EXPECT(false, equal_to(true));
    }
    catch(const expectation_error &e) {
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
    }
    catch(const expectation_error &e) {
      message = e.what();
    }
    std::ostringstream ss;
    ss << "description (" << __FILE__ << ":" << line
       << ")\nexpected: true\nactual:   false";
    expect(message, equal_to(ss.str()));
  });
});
