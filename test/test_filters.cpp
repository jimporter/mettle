#include <mettle.hpp>
using namespace mettle;

#include "helpers.hpp"
#include "../src/libmettle/filters.hpp"

suite<> test_filter("attribute filtering", [](auto &_) {
  subsuite<>(_, "filter_item", [](auto &_) {
    subsuite<>(_, "has_attr(name)", [](auto &_) {
      _.test("basic", []() {
        auto filter = has_attr("attribute");
        expect(filter.attribute, equal_to("attribute"));
        expect(filter.func(nullptr), equal_to(false));
      });

      _.test("bool_attr", []() {
        constexpr bool_attr attr("attribute");
        auto filter = has_attr("attribute");

        attr_instance a1 = attr;
        expect(filter.func(&a1), equal_to(true));

        attr_instance a2 = attr("value");
        expect(filter.func(&a2), equal_to(true));
      });

      _.test("string_attr", []() {
        constexpr string_attr attr("attribute");
        auto filter = has_attr("attribute");

        attr_instance a = attr("value");
        expect(filter.func(&a), equal_to(true));
      });

      _.test("list_attr", []() {
        constexpr list_attr attr("attribute");
        auto filter = has_attr("attribute");

        attr_instance a = attr("value");
        expect(filter.func(&a), equal_to(true));
      });
    });

    subsuite<>(_, "has_attr(name, value)", [](auto &_) {
      _.test("basic", []() {
        auto filter = has_attr("attribute", "value");
        expect(filter.attribute, equal_to("attribute"));
        expect(filter.func(nullptr), equal_to(false));
      });

      _.test("bool_attr", []() {
        constexpr bool_attr attr("attribute");
        auto filter = has_attr("attribute", "value");

        attr_instance a1 = attr;
        expect(filter.func(&a1), equal_to(false));

        attr_instance a2 = attr("value");
        expect(filter.func(&a2), equal_to(true));
        attr_instance a3 = attr("other");
        expect(filter.func(&a3), equal_to(false));
      });

      _.test("string_attr", []() {
        constexpr string_attr attr("attribute");
        auto filter = has_attr("attribute", "value");

        attr_instance a1 = attr("value");
        expect(filter.func(&a1), equal_to(true));
        attr_instance a2 = attr("other");
        expect(filter.func(&a2), equal_to(false));
      });

      _.test("list_attr", []() {
        constexpr list_attr attr("attribute");
        auto filter = has_attr("attribute", "value");

        attr_instance a1 = attr("value");
        expect(filter.func(&a1), equal_to(true));
        attr_instance a2 = attr("other");
        expect(filter.func(&a2), equal_to(false));
        attr_instance a3 = attr("other", "value");
        expect(filter.func(&a3), equal_to(true));
      });
    });
  });

  subsuite<>(_, "attr_filter", [](auto &_) {
    _.test("empty filter", []() {
      constexpr bool_attr attr("bool");
      expect(
        attr_filter{}( {} ),
        equal_to(filter_result{attr_action::run, nullptr})
      );
      expect(
        attr_filter{}( {attr} ),
        equal_to(filter_result{attr_action::run, nullptr})
      );
    });

    _.test("matching bool filter", []() {
      constexpr bool_attr attr1("first");
      constexpr bool_attr attr2("second");

      expect(
        attr_filter{ has_attr("first") }( {attr1} ),
        equal_to(filter_result{attr_action::run, nullptr})
      );
      expect(
        attr_filter{ has_attr("first"), has_attr("second") }( {attr1, attr2} ),
        equal_to(filter_result{attr_action::run, nullptr})
      );
    });

    _.test("matching value filter", []() {
      constexpr string_attr attr1("first");
      constexpr string_attr attr2("second");

      expect(
        attr_filter{ has_attr("first", "value") }( {attr1("value")} ),
        equal_to(filter_result{attr_action::run, nullptr})
      );
      expect(
        attr_filter{ has_attr("first", "1"), has_attr("second", "2") }({
          attr1("1"), attr2("2")
        }),
        equal_to(filter_result{attr_action::run, nullptr})
      );
    });

    _.test("non-matching bool filter", []() {
      constexpr bool_attr attr("bool");

      expect(
        attr_filter{ has_attr("mismatch") }( {} ),
        equal_to(filter_result{attr_action::hide, nullptr})
      );
      expect(
        attr_filter{ has_attr("mismatch") }( {attr} ),
        equal_to(filter_result{attr_action::hide, nullptr})
      );
      expect(
        attr_filter{ has_attr("mismatch"), has_attr("bool") }( {attr} ),
        equal_to(filter_result{attr_action::hide, nullptr})
      );
    });

    _.test("non-matching value filter", []() {
      constexpr string_attr attr("string");

      expect(
        attr_filter{ has_attr("mismatch", "value") }( {} ),
        equal_to(filter_result{attr_action::hide, nullptr})
      );
      expect(
        attr_filter{ has_attr("mismatch", "value") }( {attr("value")} ),
        equal_to(filter_result{attr_action::hide, nullptr})
      );
      attr_list attrs = { attr("value") };
      expect(
        attr_filter{ has_attr("string", "mismatch") }(attrs),
        equal_to(filter_result{attr_action::hide, &*attrs.begin()})
      );
    });

    _.test("explicitly matching skipped attr", []() {
      constexpr bool_attr attr1("first", attr_action::skip);
      constexpr bool_attr attr2("second", attr_action::skip);

      expect(
        attr_filter{ has_attr("first") }( {attr1} ),
        equal_to(filter_result{attr_action::run, nullptr})
      );
      expect(
        attr_filter{ has_attr("first"), has_attr("second") }( {attr1, attr2} ),
        equal_to(filter_result{attr_action::run, nullptr})
      );
    });

    _.test("implicitly matching skipped attr", []() {
      constexpr bool_attr attr1("first", attr_action::skip);
      constexpr bool_attr attr2("second");
      constexpr bool_attr attr3("third");

      attr_list attrs = { attr1, attr2, attr3 };
      expect(
        attr_filter{}(attrs),
        equal_to(filter_result{attr_action::skip, &*attrs.begin()})
      );
      expect(
        attr_filter{ has_attr("second") }(attrs),
        equal_to(filter_result{attr_action::skip, &*attrs.begin()})
      );
      expect(
        attr_filter{ has_attr("second"), has_attr("third") }(attrs),
        equal_to(filter_result{attr_action::skip, &*attrs.begin()})
      );
    });

    _.test("not matching skipped attr", []() {
      constexpr bool_attr attr1("first", attr_action::skip);
      constexpr bool_attr attr2("second", attr_action::skip);

      expect(
        attr_filter{ has_attr("mismatch") }( {attr1} ),
        equal_to(filter_result{attr_action::hide, nullptr})
      );
      expect(
        attr_filter{ has_attr("first"), has_attr("mismatch") }({attr1, attr2}),
        equal_to(filter_result{attr_action::hide, nullptr})
      );
    });
  });

  subsuite<>(_, "attr_filter_set", [](auto &_) {
    _.test("empty set", []() {
      constexpr bool_attr attr1("first", attr_action::skip);
      constexpr bool_attr attr2("second");

      expect(
        attr_filter_set{}( {} ),
        equal_to(filter_result{attr_action::run, nullptr})
      );
      expect(
        attr_filter_set{}( {attr2} ),
        equal_to(filter_result{attr_action::run, nullptr})
      );
      attr_list attrs = { attr1 };
      expect(
        attr_filter_set{}(attrs),
        equal_to(filter_result{attr_action::skip, &*attrs.begin()})
      );
    });

    _.test("single filter", []() {
      constexpr bool_attr attr1("first", attr_action::skip);
      constexpr bool_attr attr2("second");

      expect(
        attr_filter_set{ {has_attr("first")} }( {} ),
        equal_to(filter_result{attr_action::hide, nullptr})
      );
      expect(
        attr_filter_set{ {has_attr("first")} }( {attr1} ),
        equal_to(filter_result{attr_action::run, nullptr})
      );
      attr_list attrs = { attr1, attr2 };
      expect(
        attr_filter_set{ {has_attr("second")} }(attrs),
        equal_to(filter_result{attr_action::skip, &*attrs.begin()})
      );
    });

    _.test("multiple filters", []() {
      constexpr bool_attr attr1("first", attr_action::skip);
      constexpr bool_attr attr2("second");

      attr_list both_attrs = { attr1, attr2 };

      // hide + hide => hide
      expect(
        attr_filter_set{ {has_attr("first")}, {has_attr("second")} }( {} ),
        equal_to(filter_result{attr_action::hide, nullptr})
      );

      // run + run => run
      expect(
        attr_filter_set{{has_attr("first")}, {has_attr("second")}}(both_attrs),
        equal_to(filter_result{attr_action::run, nullptr})
      );

      // skip + skip => skip
      expect(
        attr_filter_set{{has_attr("second")}, {has_attr("second")}}(both_attrs),
        equal_to(filter_result{attr_action::skip, &*both_attrs.begin()})
      );

      // hide + skip => skip
      expect(
        attr_filter_set{{has_attr("other")}, {has_attr("second")}}(both_attrs),
        equal_to(filter_result{attr_action::skip, &*both_attrs.begin()})
      );

      // hide + run => run
      expect(
        attr_filter_set{{has_attr("other")}, {has_attr("second")}}( {attr2} ),
        equal_to(filter_result{attr_action::run, nullptr})
      );

      // run + skip => run
      expect(
        attr_filter_set{{has_attr("first")}, {has_attr("second")}}(both_attrs),
        equal_to(filter_result{attr_action::run, nullptr})
      );
    });
  });
});
