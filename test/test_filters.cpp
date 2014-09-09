#include <mettle.hpp>
using namespace mettle;

#include "helpers.hpp"
#include "../src/libmettle/filters.hpp"

auto name_filter_suite(bool negated) {
  auto filter = has_attr("attribute");
  std::string suite_name("has_attr(name)");
  if(negated) {
    filter = !std::move(filter);
    suite_name = "!" + suite_name;
  }

  return make_subsuite<std::tuple<>>(suite_name, [negated, filter](auto &_) {
    _.test("basic", [=]() {
      expect(filter.attribute, equal_to("attribute"));
      expect(filter.func(nullptr), equal_to(negated));
    });

    _.test("bool_attr", [=]() {
      constexpr bool_attr attr("attribute");

      attr_instance a1 = attr;
      expect(filter.func(&a1), equal_to(!negated));

      attr_instance a2 = attr("value");
      expect(filter.func(&a2), equal_to(!negated));
    });

    _.test("string_attr", [=]() {
      constexpr string_attr attr("attribute");

      attr_instance a = attr("value");
      expect(filter.func(&a), equal_to(!negated));
    });

    _.test("list_attr", [=]() {
      constexpr list_attr attr("attribute");

      attr_instance a = attr("value");
      expect(filter.func(&a), equal_to(!negated));
    });
  });
}

auto value_filter_suite(bool negated) {
  auto filter = has_attr("attribute", "value");
  std::string suite_name("has_attr(name, value)");
  if(negated) {
    filter = !std::move(filter);
    suite_name = "!" + suite_name;
  }

  return make_subsuite<std::tuple<>>(suite_name, [negated, filter](auto &_) {
    _.test("basic", [=]() {
      expect(filter.attribute, equal_to("attribute"));
      expect(filter.func(nullptr), equal_to(negated));
    });

    _.test("bool_attr", [=]() {
      constexpr bool_attr attr("attribute");

      attr_instance a1 = attr;
      expect(filter.func(&a1), equal_to(negated));

      attr_instance a2 = attr("value");
      expect(filter.func(&a2), equal_to(!negated));
      attr_instance a3 = attr("other");
      expect(filter.func(&a3), equal_to(negated));
    });

    _.test("string_attr", [=]() {
      constexpr string_attr attr("attribute");

      attr_instance a1 = attr("value");
      expect(filter.func(&a1), equal_to(!negated));
      attr_instance a2 = attr("other");
      expect(filter.func(&a2), equal_to(negated));
    });

    _.test("list_attr", [=]() {
      constexpr list_attr attr("attribute");

      attr_instance a1 = attr("value");
      expect(filter.func(&a1), equal_to(!negated));
      attr_instance a2 = attr("other");
      expect(filter.func(&a2), equal_to(negated));
      attr_instance a3 = attr("other", "value");
      expect(filter.func(&a3), equal_to(!negated));
    });
  });
}

suite<> test_filter("attribute filtering", [](auto &_) {
  subsuite<>(_, "filter_item", [](auto &_) {
    for(bool negated : {false, true}) {
      _.subsuite(name_filter_suite(negated));
      _.subsuite(value_filter_suite(negated));
    }
  });

  subsuite<>(_, "attr_filter", [](auto &_) {
    subsuite<>(_, "matching filters", [](auto &_) {
      _.test("empty", []() {
        constexpr bool_attr attr("bool");
        expect(
          attr_filter{}( {} ),
          equal_to(filter_result{test_action::run, ""})
        );
        expect(
          attr_filter{}( {attr} ),
          equal_to(filter_result{test_action::run, ""})
        );
      });

      _.test("has_attr(name)", []() {
        constexpr bool_attr attr1("first");
        constexpr bool_attr attr2("second");

        expect(
          attr_filter{ has_attr("first") }( {attr1} ),
          equal_to(filter_result{test_action::run, ""})
        );
        expect(
          attr_filter{ has_attr("first") }( {attr1, attr2} ),
          equal_to(filter_result{test_action::run, ""})
        );
        expect(
          attr_filter{ has_attr("first"), has_attr("second") }(
            { attr1, attr2 }
          ),
          equal_to(filter_result{test_action::run, ""})
        );
      });

      _.test("has_attr(name, value)", []() {
        constexpr string_attr attr1("first");
        constexpr string_attr attr2("second");

        expect(
          attr_filter{ has_attr("first", "1") }( {attr1("1")} ),
          equal_to(filter_result{test_action::run, ""})
        );
        expect(
          attr_filter{ has_attr("first", "1") }( {attr1("1"), attr2("2")} ),
          equal_to(filter_result{test_action::run, ""})
        );
        expect(
          attr_filter{ has_attr("first", "1"), has_attr("second", "2") }(
            { attr1("1"), attr2("2") }
          ),
          equal_to(filter_result{test_action::run, ""})
        );
      });

      _.test("!has_attr(name)", []() {
        constexpr bool_attr attr("bool");

        expect(
          attr_filter{ !has_attr("mismatch") }( {} ),
          equal_to(filter_result{test_action::run, ""})
        );
        expect(
          attr_filter{ !has_attr("mismatch") }( {attr} ),
          equal_to(filter_result{test_action::run, ""})
        );
        expect(
          attr_filter{ !has_attr("mismatch"), has_attr("bool") }( {attr} ),
          equal_to(filter_result{test_action::run, ""})
        );
      });

      _.test("!has_attr(name, value)", []() {
        constexpr string_attr attr("string");

        expect(
          attr_filter{ !has_attr("mismatch", "value") }( {} ),
          equal_to(filter_result{test_action::run, ""})
        );
        expect(
          attr_filter{ !has_attr("mismatch", "value") }( {attr("value")} ),
          equal_to(filter_result{test_action::run, ""})
        );
        expect(
          attr_filter{ !has_attr("string", "mismatch") }( {attr("value")} ),
          equal_to(filter_result{test_action::run, ""})
        );
      });

      _.test("skipped attr, explicit", []() {
        constexpr bool_attr attr1("first", test_action::skip);
        constexpr bool_attr attr2("second", test_action::skip);

        expect(
          attr_filter{ has_attr("first") }( {attr1} ),
          equal_to(filter_result{test_action::run, ""})
        );
        expect(
          attr_filter{ has_attr("first"), has_attr("second") }(
            {attr1, attr2}
          ),
          equal_to(filter_result{test_action::run, ""})
        );
      });

      _.test("skipped attr, implicit", []() {
        constexpr bool_attr attr1("first", test_action::skip);
        constexpr bool_attr attr2("second");
        constexpr bool_attr attr3("third");

        attributes attrs = { attr1("message"), attr2, attr3 };
        expect(
          attr_filter{}(attrs),
          equal_to(filter_result{test_action::skip, "message"})
        );
        expect(
          attr_filter{ has_attr("second") }(attrs),
          equal_to(filter_result{test_action::skip, "message"})
        );
        expect(
          attr_filter{ has_attr("second"), has_attr("third") }(attrs),
          equal_to(filter_result{test_action::skip, "message"})
        );
      });
    });

    subsuite<>(_, "non-matching filters", [](auto &_) {
      _.test("has_attr(name)", []() {
        constexpr bool_attr attr("bool");

        expect(
          attr_filter{ has_attr("mismatch") }( {} ),
          equal_to(filter_result{test_action::hide, ""})
        );
        expect(
          attr_filter{ has_attr("mismatch") }( {attr} ),
          equal_to(filter_result{test_action::hide, ""})
        );
        expect(
          attr_filter{ has_attr("mismatch"), has_attr("bool") }( {attr} ),
          equal_to(filter_result{test_action::hide, ""})
        );
      });

      _.test("has_attr(name, value)", []() {
        constexpr string_attr attr("string");

        expect(
          attr_filter{ has_attr("mismatch", "value") }( {} ),
          equal_to(filter_result{test_action::hide, ""})
        );
        expect(
          attr_filter{ has_attr("mismatch", "value") }( {attr("value")} ),
          equal_to(filter_result{test_action::hide, ""})
        );
        attributes attrs = { attr("value") };
        expect(
          attr_filter{ has_attr("string", "mismatch") }(attrs),
          equal_to(filter_result{test_action::hide, "value"})
        );
      });

      _.test("!has_attr(name)", []() {
        constexpr bool_attr attr1("first");
        constexpr bool_attr attr2("second");

        attributes first_attr = { attr1("1") };
        attributes both_attrs = { attr1("1"), attr2("2") };

        expect(
          attr_filter{ !has_attr("first") }(first_attr),
          equal_to(filter_result{test_action::hide, "1"})
        );
        expect(
          attr_filter{ !has_attr("first") }(both_attrs),
          equal_to(filter_result{test_action::hide, "1"})
        );
        expect(
          attr_filter{ !has_attr("first"), !has_attr("second") }(both_attrs),
          equal_to(filter_result{test_action::hide, "1"})
        );
        expect(
          attr_filter{ !has_attr("first"), has_attr("second") }(both_attrs),
          equal_to(filter_result{test_action::hide, "1"})
        );
        expect(
          attr_filter{ has_attr("first"), !has_attr("second") }(both_attrs),
          equal_to(filter_result{test_action::hide, "2"})
        );
      });

      _.test("!has_attr(name, value)", []() {
        constexpr string_attr attr1("first");
        constexpr string_attr attr2("second");

        attributes first_attr = { attr1("1") };
        attributes both_attrs = { attr1("1"), attr2("2") };

        expect(
          attr_filter{ !has_attr("first", "1") }(first_attr),
          equal_to(filter_result{test_action::hide, "1"})
        );
        expect(
          attr_filter{ !has_attr("first", "1") }(both_attrs),
          equal_to(filter_result{test_action::hide, "1"})
        );
        expect(
          attr_filter{ !has_attr("first", "1"), !has_attr("second", "2") }(
            both_attrs
          ),
          equal_to(filter_result{test_action::hide, "1"})
        );
        expect(
          attr_filter{ !has_attr("first", "1"), has_attr("second", "2") }(
            both_attrs
          ),
          equal_to(filter_result{test_action::hide, "1"})
        );
        expect(
          attr_filter{ has_attr("first", "1"), !has_attr("second", "2") }(
            both_attrs
          ),
          equal_to(filter_result{test_action::hide, "2"})
        );
      });

      _.test("skipped attr", []() {
        constexpr bool_attr attr1("first", test_action::skip);
        constexpr bool_attr attr2("second", test_action::skip);

        expect(
          attr_filter{ has_attr("mismatch") }( {attr1} ),
          equal_to(filter_result{test_action::hide, ""})
        );
        expect(
          attr_filter{ has_attr("first"), has_attr("mismatch") }(
            { attr1, attr2 }
          ),
          equal_to(filter_result{test_action::hide, ""})
        );
      });
    });
  });

  subsuite<>(_, "attr_filter_set", [](auto &_) {
    _.test("empty set", []() {
      constexpr bool_attr attr1("first", test_action::skip);
      constexpr bool_attr attr2("second");

      expect(
        attr_filter_set{}( {} ),
        equal_to(filter_result{test_action::run, ""})
      );
      expect(
        attr_filter_set{}( {attr2} ),
        equal_to(filter_result{test_action::run, ""})
      );
      attributes attrs = { attr1("message") };
      expect(
        attr_filter_set{}(attrs),
        equal_to(filter_result{test_action::skip, "message"})
      );
    });

    _.test("single filter", []() {
      constexpr bool_attr attr1("first", test_action::skip);
      constexpr bool_attr attr2("second");

      expect(
        attr_filter_set{ {has_attr("first")} }( {} ),
        equal_to(filter_result{test_action::hide, ""})
      );
      expect(
        attr_filter_set{ {!has_attr("first")} }( {} ),
        equal_to(filter_result{test_action::run, ""})
      );
      expect(
        attr_filter_set{ {has_attr("first")} }( {attr1} ),
        equal_to(filter_result{test_action::run, ""})
      );
      attributes attrs = { attr1("1"), attr2("2") };
      expect(
        attr_filter_set{ {has_attr("second")} }(attrs),
        equal_to(filter_result{test_action::skip, "1"})
      );
      expect(
        attr_filter_set{ {!has_attr("first")} }(attrs),
        equal_to(filter_result{test_action::hide, "1"})
      );
    });

    _.test("multiple filters", []() {
      constexpr bool_attr attr1("first", test_action::skip);
      constexpr bool_attr attr2("second");

      attributes both_attrs = { attr1("1"), attr2("2") };

      // hide + hide => hide
      expect(
        attr_filter_set{ {has_attr("first")}, {has_attr("second")} }( {} ),
        equal_to(filter_result{test_action::hide, ""})
      );
      expect(
        attr_filter_set{ {!has_attr("first")}, {!has_attr("second")} }(
          both_attrs
        ),
        equal_to(filter_result{test_action::hide, "1"})
      );

      // run + run => run
      expect(
        attr_filter_set{{has_attr("first")}, {has_attr("second")}}(both_attrs),
        equal_to(filter_result{test_action::run, ""})
      );

      // skip + skip => skip
      expect(
        attr_filter_set{{has_attr("second")}, {has_attr("second")}}(both_attrs),
        equal_to(filter_result{test_action::skip, "1"})
      );

      // hide + skip => skip
      expect(
        attr_filter_set{{has_attr("other")}, {has_attr("second")}}(both_attrs),
        equal_to(filter_result{test_action::skip, "1"})
      );

      // hide + run => run
      expect(
        attr_filter_set{{has_attr("other")}, {has_attr("second")}}( {attr2} ),
        equal_to(filter_result{test_action::run, ""})
      );

      // run + skip => run
      expect(
        attr_filter_set{{has_attr("first")}, {has_attr("second")}}(both_attrs),
        equal_to(filter_result{test_action::run, ""})
      );
    });
  });
});
