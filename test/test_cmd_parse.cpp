#include <mettle.hpp>
using namespace mettle;

#include <mettle/driver/cmd_line.hpp>
#include "helpers.hpp"

auto match_filter_item(const attr_instance &attr, bool matched) {
  std::ostringstream ss;
  ss << "filter_item(" << to_printable(attr.attribute.name()) << ") => "
     << to_printable(matched);
  return make_matcher([attr, matched](const attr_filter_item &actual) {
    return actual.attribute == attr.attribute.name() &&
           actual.func(&attr) == matched;
  }, ss.str());
}

namespace mettle {
  std::string to_printable(const attr_filter_item &item) {
    return "filter_item(" + to_printable(item.attribute) + ")";
  }
}

suite<> test_parse_attr("parse attributes", [](auto &_) {
  _.test("attr", []() {
    bool_attr attr("attr");

    auto filter = parse_attr("attr");
    expect(filter.size(), equal_to(1u));
    expect(filter, array( match_filter_item(attr, true) ));
    expect(filter(test_name(), {attr}),
           equal_to( filter_result{test_action::run, ""}) );
  });

  _.test("attr=value", []() {
    string_attr attr("attr");

    auto filter = parse_attr("attr=value");
    expect(filter.size(), equal_to(1u));
    expect(filter, array( match_filter_item(attr("value"), true) ));
    expect(filter(test_name(), {attr("value")}),
           equal_to( filter_result{test_action::run, ""} ));
  });

  _.test("attr=", []() {
    string_attr attr("attr");

    auto filter = parse_attr("attr=");
    expect(filter.size(), equal_to(1u));
    expect(filter, array( match_filter_item(attr(""), true) ));
    expect(filter(test_name(), {attr("")}),
           equal_to( filter_result{test_action::run, ""} ));
  });

  _.test("!attr", []() {
    bool_attr attr("attr");

    auto filter = parse_attr("!attr");
    expect(filter.size(), equal_to(1u));
    expect(filter, array( match_filter_item(attr, false) ));
    attributes attrs = {attr("message")};
    expect(filter(test_name(), attrs),
           equal_to( filter_result{test_action::hide, "message"}) );
  });

  _.test("!attr=value", []() {
    string_attr attr("attr");

    auto filter = parse_attr("!attr=value");
    expect(filter.size(), equal_to(1u));
    expect(filter, array( match_filter_item(attr("value"), false) ));
    attributes attrs = {attr("value")};
    expect(filter(test_name(), attrs),
           equal_to( filter_result{test_action::hide, "value"} ));
  });

  _.test("!attr=", []() {
    string_attr attr("attr");

    auto filter = parse_attr("!attr=");
    expect(filter.size(), equal_to(1u));
    expect(filter, array( match_filter_item(attr(""), false) ));
    attributes attrs = {attr("")};
    expect(filter(test_name(), attrs),
           equal_to( filter_result{test_action::hide, ""} ));
  });

  _.test("attr1,attr2", []() {
    bool_attr attr1("attr1");
    bool_attr attr2("attr2");

    auto filter = parse_attr("attr1,attr2");
    expect(filter.size(), equal_to(2u));
    expect(filter, array(
      match_filter_item(attr1, true), match_filter_item(attr2, true)
    ));
    expect(filter(test_name(), {attr1, attr2}),
           equal_to( filter_result{test_action::run, ""}) );
  });

  _.test("attr1=1,attr2=2", []() {
    string_attr attr1("attr1");
    string_attr attr2("attr2");

    auto filter = parse_attr("attr1=1,attr2=2");
    expect(filter.size(), equal_to(2u));
    expect(filter, array(
      match_filter_item(attr1("1"), true), match_filter_item(attr2("2"), true)
    ));
    expect(filter(test_name(), {attr1("1"), attr2("2")}),
           equal_to( filter_result{test_action::run, ""}) );
  });

  _.test("attr1,!attr2", []() {
    bool_attr attr1("attr1");
    bool_attr attr2("attr2");

    auto filter = parse_attr("attr1,!attr2");
    expect(filter.size(), equal_to(2u));
    expect(filter, array(
      match_filter_item(attr1, true), match_filter_item(attr2, false)
    ));
    attributes attrs = {attr1, attr2};
    expect(filter(test_name(), attrs),
           equal_to( filter_result{test_action::hide, ""}) );
  });

  _.test("attr1=1,!attr2=2", []() {
    string_attr attr1("attr1");
    string_attr attr2("attr2");

    auto filter = parse_attr("attr1=1,!attr2=2");
    expect(filter.size(), equal_to(2u));
    expect(filter, array(
      match_filter_item(attr1("1"), true), match_filter_item(attr2("2"), false)
    ));
    attributes attrs = {attr1("1"), attr2("2")};
    expect(filter(test_name(), attrs),
           equal_to( filter_result{test_action::hide, "2"}) );
  });

  _.test("parse failures", []() {
    expect([]() {parse_attr(""); },
           thrown<std::invalid_argument>("unexpected end of string"));
    expect([]() {parse_attr("!"); },
           thrown<std::invalid_argument>("unexpected end of string"));
    expect([]() {parse_attr(","); },
           thrown<std::invalid_argument>("expected attribute name"));
    expect([]() {parse_attr("="); },
           thrown<std::invalid_argument>("expected attribute name"));
    expect([]() {parse_attr("attr,"); },
           thrown<std::invalid_argument>("unexpected end of string"));
    expect([]() {parse_attr("attr,,"); },
           thrown<std::invalid_argument>("expected attribute name"));
    expect([]() {parse_attr("attr,="); },
           thrown<std::invalid_argument>("expected attribute name"));
  });
});
