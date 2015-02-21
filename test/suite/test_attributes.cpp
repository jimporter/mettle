#include <mettle.hpp>
using namespace mettle;

#include "../helpers.hpp"

suite<> test_attr("attributes", [](auto &_) {
  _.test("unite(attr_instance, attr_instance) checks attributes", []() {
      bool_attr attr1("attribute");
      bool_attr attr2("attribute");
      expect(
        [&attr1, &attr2]() { unite(attr1, attr2); },
        thrown<std::invalid_argument>("mismatched attributes")
      );
  });

  subsuite<>(_, "bool_attr", [](auto &_) {
    _.test("without comment", []() {
      bool_attr attr("attribute");
      attr_instance a = attr;

      expect(&a.attribute, equal_to(&attr));
      expect(a.value, array());
    });

    _.test("with comment", []() {
      bool_attr attr("attribute");
      attr_instance a = attr("comment");

      expect(&a.attribute, equal_to(&attr));
      expect(a.value, array("comment"));
    });

    _.test("skipped attribute", []() {
      bool_attr attr("attribute", test_action::skip);
      attr_instance a = attr;

      expect(&a.attribute, equal_to(&attr));
      expect(a.value, array());
    });

    _.test("unite()", []() {
      bool_attr attr("attribute");
      attr_instance a = unite(attr("a"), attr("b"));

      expect(&a.attribute, equal_to(&attr));
      expect(a.value, array("a"));
    });
  });

  subsuite<>(_, "string_attr", [](auto &_) {
    _.test("with value", []() {
      string_attr attr("attribute");
      attr_instance a = attr("value");

      expect(&a.attribute, equal_to(&attr));
      expect(a.value, array("value"));
    });

    _.test("unite()", []() {
      string_attr attr("attribute");
      attr_instance a = unite(attr("a"), attr("b"));

      expect(&a.attribute, equal_to(&attr));
      expect(a.value, array("a"));
    });
  });

  subsuite<>(_, "list_attr", [](auto &_) {
    _.test("single value", []() {
      list_attr attr("attribute");
      attr_instance a = attr("value");

      expect(&a.attribute, equal_to(&attr));
      expect(a.value, array("value"));
    });

    _.test("multiple values", []() {
      list_attr attr("attribute");
      attr_instance a = attr("value 1", "value 2");

      expect(&a.attribute, equal_to(&attr));
      expect(a.value, array("value 1", "value 2"));
    });

    _.test("unite()", []() {
      list_attr attr("attribute");
      attr_instance a = unite(attr("a"), attr("b"));

      expect(&a.attribute, equal_to(&attr));
      expect(a.value, array("a", "b"));
    });
  });

  subsuite<>(_, "unite(attributes, attributes)", [](auto &_) {
    _.test("empty sets", []() {
      string_attr attr("1");

      expect(unite({}, {}), equal_attributes({}));
      expect(unite({attr("a")}, {}), equal_attributes( {attr("a")} ));
      expect(unite({}, {attr("b")}), equal_attributes( {attr("b")} ));
    });

    _.test("disjoint sets", []() {
      string_attr attr1("1");
      string_attr attr2("2");
      string_attr attr3("3");

      auto united = unite(
        {attr1("a")},
        {attr2("b"), attr3("b")}
      );
      expect(united, equal_attributes( {attr1("a"), attr2("b"), attr3("b")} ));
    });

    _.test("intersecting sets", []() {
      string_attr attr1("1");
      string_attr attr2("2");
      string_attr attr3("3");

      auto united = unite(
        {attr1("a"), attr2("a")},
        {attr2("b"), attr3("b")}
      );
      expect(united, equal_attributes( {attr1("a"), attr2("a"), attr3("b")} ));
    });

    _.test("intersecting sets (composable attrs)", []() {
      list_attr attr1("1");
      list_attr attr2("2");
      list_attr attr3("3");

      auto united = unite(
        {attr1("a"), attr2("a")},
        {attr2("b"), attr3("b")}
      );
      expect(united, equal_attributes({
        attr1("a"), attr2("a", "b"), attr3("b")
      }));
    });
  });
});
