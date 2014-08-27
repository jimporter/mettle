#include <mettle.hpp>
using namespace mettle;

namespace mettle {
  std::string ensure_printable(const attr_instance &attr) {
    std::stringstream s;
    s << attr.name();
    if(!attr.empty()) {
      auto i = attr.value().begin();
      s << "(" << ensure_printable(*i);
      for(++i; i != attr.value().end(); ++i)
        s << ", " << ensure_printable(*i);
      s << ")";
    }
    return s.str();
  }

  std::string ensure_printable(const attr_list &attrs) {
    std::stringstream s;
    s << "{";
    if(!attrs.empty()) {
      auto i = attrs.begin();
      s << ensure_printable(*i);
      for(++i; i != attrs.end(); ++i)
        s << ", " << ensure_printable(*i);
    }
    s << "}";
    return s.str();
  }

  std::string ensure_printable(const attr_action &action) {
    switch(action) {
    case attr_action::run:
      return "attr_action::run";
    case attr_action::skip:
      return "attr_action::skip";
    case attr_action::hide:
      return "attr_action::hide";
    }
  }

  bool operator ==(const attr_instance &lhs, const attr_instance &rhs) {
    return lhs.name() == rhs.name() && std::equal(
      lhs.value().begin(), lhs.value().end(),
      rhs.value().begin(), rhs.value().end()
    );
  }

  bool operator ==(const attr_list &lhs, const attr_list &rhs) {
    return std::equal(lhs.begin(), lhs.end(), rhs.begin(), rhs.end());
  }
}

suite<> test_attr("attribute creation", [](auto &_) {
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
});

suite<> test_filter("attribute filtering", [](auto &_) {
  using filter_result = std::pair<attr_action, const attr_instance *>;

  subsuite<>(_, "filter_item", [](auto &_) {
    subsuite<>(_, "has_attr(name)", [](auto &_) {
      _.test("basic", []() {
        auto filter = has_attr("attribute");
        expect(filter.attr, equal_to("attribute"));
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
        expect(filter.attr, equal_to("attribute"));
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
