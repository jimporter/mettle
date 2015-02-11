#include <mettle.hpp>
using namespace mettle;

#include <sstream>

struct some_type {};

suite<> test_matcher_message("matcher_message()", [](auto &_) {
  _.test("match_result with message", []() {
    std::ostringstream ss1;
    ss1 << matcher_message({true, "message"}, "value");
    expect(ss1.str(), equal_to("message"));

    std::ostringstream ss2;
    ss2 << matcher_message({true, "message"}, some_type{});
    expect(ss2.str(), equal_to("message"));
  });

  _.test("match_result without message", []() {
    std::ostringstream ss1;
    ss1 << matcher_message({true, ""}, "value");
    expect(ss1.str(), equal_to("\"value\""));

    std::ostringstream ss2;
    ss2 << matcher_message({true, ""}, some_type{});
    expect(ss2.str(), equal_to(type_name<some_type>()));
  });

  _.test("bool", []() {
    std::ostringstream ss1;
    ss1 << matcher_message(true, "value");
    expect(ss1.str(), equal_to("\"value\""));

    std::ostringstream ss2;
    ss2 << matcher_message(true, some_type{});
    expect(ss2.str(), equal_to(type_name<some_type>()));
  });
});
