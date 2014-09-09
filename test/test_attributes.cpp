#include <mettle.hpp>
using namespace mettle;

#include "helpers.hpp"

suite<> test_attr("attributes", [](auto &_) {
  _.test("unite(attr_instance, attr_instance) checks attributes", []() {
      constexpr bool_attr attr1("attribute");
      constexpr bool_attr attr2("attribute");
      expect(
        [&attr1, &attr2]() { unite(attr1, attr2); },
        thrown<std::invalid_argument>("mismatched attributes")
      );
  });

  subsuite<>(_, "bool_attr", [](auto &_) {
    _.test("without comment", []() {
      constexpr bool_attr attr("attribute");
      attr_instance a = attr;

      expect(&a.attribute, equal_to(&attr));
      expect(a.value, array());
    });

    _.test("with comment", []() {
      constexpr bool_attr attr("attribute");
      attr_instance a = attr("comment");

      expect(&a.attribute, equal_to(&attr));
      expect(a.value, array("comment"));
    });

    _.test("skipped attribute", []() {
      constexpr bool_attr attr("attribute", test_action::skip);
      attr_instance a = attr;

      expect(&a.attribute, equal_to(&attr));
      expect(a.value, array());
    });

    _.test("hidden attribute fails", []() {
      expect(
        []() { bool_attr("attribute", test_action::hide); },
        thrown<std::invalid_argument>(
          "attribute's action must be \"run\" or \"skip\""
        )
      );
    });

    _.test("unite()", []() {
      constexpr bool_attr attr("attribute");
      attr_instance a = unite(attr("a"), attr("b"));

      expect(&a.attribute, equal_to(&attr));
      expect(a.value, array("a"));
    });
  });

  subsuite<>(_, "string_attr", [](auto &_) {
    _.test("with value", []() {
      constexpr string_attr attr("attribute");
      attr_instance a = attr("value");

      expect(&a.attribute, equal_to(&attr));
      expect(a.value, array("value"));
    });

    _.test("unite()", []() {
      constexpr string_attr attr("attribute");
      attr_instance a = unite(attr("a"), attr("b"));

      expect(&a.attribute, equal_to(&attr));
      expect(a.value, array("a"));
    });
  });

  subsuite<>(_, "list_attr", [](auto &_) {
    _.test("single value", []() {
      constexpr list_attr attr("attribute");
      attr_instance a = attr("value");

      expect(&a.attribute, equal_to(&attr));
      expect(a.value, array("value"));
    });

    _.test("multiple values", []() {
      constexpr list_attr attr("attribute");
      attr_instance a = attr("value 1", "value 2");

      expect(&a.attribute, equal_to(&attr));
      expect(a.value, array("value 1", "value 2"));
    });

    _.test("unite()", []() {
      constexpr list_attr attr("attribute");
      attr_instance a = unite(attr("a"), attr("b"));

      expect(&a.attribute, equal_to(&attr));
      expect(a.value, array("a", "b"));
    });
  });

  subsuite<>(_, "unite(attributes, attributes)", [](auto &_) {
    _.test("empty sets", []() {
      constexpr string_attr attr("1");

      expect(unite({}, {}), equal_to(attributes{}));
      expect(unite({attr("a")}, {}), equal_to(attributes{attr("a")}));
      expect(unite({}, {attr("b")}), equal_to(attributes{attr("b")}));
    });

    _.test("disjoint sets", []() {
      constexpr string_attr attr1("1");
      constexpr string_attr attr2("2");
      constexpr string_attr attr3("3");

      auto united = unite(
        {attr1("a")},
        {attr2("b"), attr3("b")}
      );
      expect(united, equal_to(attributes{attr1("a"), attr2("b"), attr3("b")}));
    });

    _.test("intersecting sets", []() {
      constexpr string_attr attr1("1");
      constexpr string_attr attr2("2");
      constexpr string_attr attr3("3");

      auto united = unite(
        {attr1("a"), attr2("a")},
        {attr2("b"), attr3("b")}
      );
      expect(united, equal_to(attributes{attr1("a"), attr2("a"), attr3("b")}));
    });

    _.test("intersecting sets (composable attrs)", []() {
      constexpr list_attr attr1("1");
      constexpr list_attr attr2("2");
      constexpr list_attr attr3("3");

      auto united = unite(
        {attr1("a"), attr2("a")},
        {attr2("b"), attr3("b")}
      );
      expect(united, equal_to(attributes{
        attr1("a"), attr2("a", "b"), attr3("b")
      }));
    });
  });
});
