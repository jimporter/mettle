#include <mettle.hpp>
using namespace mettle;

#include "helpers.hpp"
#include "../src/libmettle/cmd_parse.hpp"

auto match_filter_item(const attr_instance &attr, bool matched) {
  std::stringstream s;
  s << "filter_item(" << ensure_printable(attr.attribute.name()) << ") => "
    << ensure_printable(matched);
  return make_matcher([attr, matched](const attr_filter::filter_item &actual) {
    return actual.attribute == attr.attribute.name() &&
           actual.func(&attr) == matched;
  }, s.str());
}

suite<> test_parse_attr("parse attributes", [](auto &_) {
  _.test("attr", []() {
    constexpr bool_attr attr("attr");

    auto filter = parse_attr("attr");
    expect(filter.size(), equal_to(1u));
    expect(filter, array( match_filter_item(attr, true) ));
    expect(filter({attr}),
           equal_to( filter_result{attr_action::run, nullptr}) );
  });

  _.test("attr=value", []() {
    constexpr string_attr attr("attr");

    auto filter = parse_attr("attr=value");
    expect(filter.size(), equal_to(1u));
    expect(filter, array( match_filter_item(attr("value"), true) ));
    expect(filter({attr("value")}),
           equal_to( filter_result{attr_action::run, nullptr} ));
  });

  _.test("!attr", []() {
    constexpr bool_attr attr("attr");

    auto filter = parse_attr("!attr");
    expect(filter.size(), equal_to(1u));
    expect(filter, array( match_filter_item(attr, false) ));
    attr_list attrs = {attr};
    expect(filter(attrs),
           equal_to( filter_result{attr_action::hide, &*attrs.begin()}) );
  });

  _.test("!attr=value", []() {
    constexpr string_attr attr("attr");

    auto filter = parse_attr("!attr=value");
    expect(filter.size(), equal_to(1u));
    expect(filter, array( match_filter_item(attr("value"), false) ));
    attr_list attrs = {attr("value")};
    expect(filter(attrs),
           equal_to( filter_result{attr_action::hide, &*attrs.begin()} ));
  });

  _.test("attr1,attr2", []() {
    constexpr bool_attr attr1("attr1");
    constexpr bool_attr attr2("attr2");

    auto filter = parse_attr("attr1,attr2");
    expect(filter.size(), equal_to(2u));
    expect(filter, array(
      match_filter_item(attr1, true), match_filter_item(attr2, true)
    ));
    expect(filter({attr1, attr2}),
           equal_to( filter_result{attr_action::run, nullptr}) );
  });

  _.test("attr1=1,attr2=2", []() {
    constexpr string_attr attr1("attr1");
    constexpr string_attr attr2("attr2");

    auto filter = parse_attr("attr1=1,attr2=2");
    expect(filter.size(), equal_to(2u));
    expect(filter, array(
      match_filter_item(attr1("1"), true), match_filter_item(attr2("2"), true)
    ));
    expect(filter({attr1("1"), attr2("2")}),
           equal_to( filter_result{attr_action::run, nullptr}) );
  });

  _.test("attr1,!attr2", []() {
    constexpr bool_attr attr1("attr1");
    constexpr bool_attr attr2("attr2");

    auto filter = parse_attr("attr1,!attr2");
    expect(filter.size(), equal_to(2u));
    expect(filter, array(
      match_filter_item(attr1, true), match_filter_item(attr2, false)
    ));
    attr_list attrs = {attr1, attr2};
    expect(filter(attrs),
           equal_to( filter_result{attr_action::hide, &*(++attrs.begin())}) );
  });

  _.test("attr1=1,!attr2=2", []() {
    constexpr string_attr attr1("attr1");
    constexpr string_attr attr2("attr2");

    auto filter = parse_attr("attr1=1,!attr2=2");
    expect(filter.size(), equal_to(2u));
    expect(filter, array(
      match_filter_item(attr1("1"), true), match_filter_item(attr2("2"), false)
    ));
    attr_list attrs = {attr1("1"), attr2("2")};
    expect(filter(attrs),
           equal_to( filter_result{attr_action::hide, &*(++attrs.begin())}) );
  });
});
