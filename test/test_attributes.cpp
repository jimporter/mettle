#include <mettle.hpp>
using namespace mettle;

#include "helpers.hpp"

suite<> test_attr("attributes", [](auto &_) {
  subsuite<>(_, "bool_attr", [](auto &_) {
    _.test("without comment", []() {
      constexpr bool_attr attr("attribute");
      attr_instance a = attr;

      expect(a.name(), equal_to("attribute"));
      expect(a.action(), equal_to(attr_action::run));
      expect(a.empty(), equal_to(true));
      expect(a.value(), array());
    });

    _.test("with comment", []() {
      constexpr bool_attr attr("attribute");
      attr_instance a = attr("comment");

      expect(a.name(), equal_to("attribute"));
      expect(a.action(), equal_to(attr_action::run));
      expect(a.empty(), equal_to(false));
      expect(a.value(), array("comment"));
    });

    _.test("skipped attribute", []() {
      constexpr bool_attr attr("attribute", attr_action::skip);
      attr_instance a = attr;

      expect(a.name(), equal_to("attribute"));
      expect(a.action(), equal_to(attr_action::skip));
    });

    _.test("hidden attribute fails", []() {
      expect([]() { bool_attr("attribute", attr_action::hide); },
             thrown<std::invalid_argument>("attr's action can't be \"hide\""));
    });
  });

  subsuite<>(_, "string_attr", [](auto &_) {
    _.test("with value", []() {
      constexpr string_attr attr("attribute");
      attr_instance a = attr("value");

      expect(a.name(), equal_to("attribute"));
      expect(a.action(), equal_to(attr_action::run));
      expect(a.empty(), equal_to(false));
      expect(a.value(), array("value"));
    });
  });

  subsuite<>(_, "list_attr", [](auto &_) {
    _.test("single value", []() {
      constexpr list_attr attr("attribute");
      attr_instance a = attr("value");

      expect(a.name(), equal_to("attribute"));
      expect(a.action(), equal_to(attr_action::run));
      expect(a.empty(), equal_to(false));
      expect(a.value(), array("value"));
    });

    _.test("multiple values", []() {
      constexpr list_attr attr("attribute");
      attr_instance a = attr("value 1", "value 2");

      expect(a.name(), equal_to("attribute"));
      expect(a.action(), equal_to(attr_action::run));
      expect(a.empty(), equal_to(false));
      expect(a.value(), array("value 1", "value 2"));
    });
  });

  subsuite<>(_, "attr_list", [](auto &_) {
    _.test("unite", []() {
      constexpr bool_attr attr1("1");
      constexpr bool_attr attr2("2");
      constexpr bool_attr attr3("3");

      attr_list a = {attr1, attr2};
      attr_list b = {attr2, attr3};
      auto united = unite(a, b);
      expect(united, equal_to(attr_list{attr1, attr2, attr3}));
    });
  });

  subsuite<>(_, "default_attr_filter", [](auto &_) {
    _.test("no attributes", []() {
      expect(
        default_attr_filter{}( {} ),
        equal_to(filter_result{attr_action::run, nullptr})
      );
    });

    _.test("regular attribute", []() {
      constexpr bool_attr attr("bool");
      expect(
        default_attr_filter{}( {attr} ),
        equal_to(filter_result{attr_action::run, nullptr})
      );
    });

    _.test("skipped attribute", []() {
      constexpr bool_attr attr("bool", attr_action::skip);
      attr_list attrs = {attr};
      expect(
        default_attr_filter{}(attrs),
        equal_to(filter_result{attr_action::skip, &*attrs.begin()})
      );
    });
  });
});
