#include <mettle.hpp>
using namespace mettle;

#include <mettle/driver/filters.hpp>
#include "../helpers.hpp"

suite<> test_core_filters("core filters", [](auto &_) {
  subsuite<>(_, "default_filter", [](auto &_) {
    _.test("no attributes", []() {
      expect(
        default_filter{}(test_name(), {}),
        equal_filter_result({test_action::indeterminate, ""})
      );
    });

    _.test("regular attribute", []() {
      bool_attr attr("bool");
      expect(
        default_filter{}(test_name(), {attr}),
        equal_filter_result({test_action::indeterminate, ""})
      );
    });

    _.test("skipped attribute", []() {
      bool_attr attr("bool", test_action::skip);
      expect(
        default_filter{}(test_name(), {attr("message")}),
        equal_filter_result({test_action::indeterminate, ""})
      );
    });
  });

  subsuite<>(_, "filter_by_attr", [](auto &_) {
    _.test("no attributes", []() {
      expect(
        filter_by_attr({}),
        equal_filter_result({test_action::run, ""})
      );
    });

    _.test("regular attribute", []() {
      bool_attr attr("bool");
      expect(
        filter_by_attr({attr}),
        equal_filter_result({test_action::run, ""})
      );
    });

    _.test("skipped attribute", []() {
      bool_attr attr("bool", test_action::skip);
      expect(
        filter_by_attr({attr("message")}),
        equal_filter_result({test_action::skip, "message"})
      );
    });
  });
});

suite<> test_name_filters("name filters", [](auto &_) {
  _.test("empty set", []() {
    expect(
      name_filter_set{}({{"suite", "subsuite"}, "test", 1}, {}),
      equal_filter_result({test_action::indeterminate, ""})
    );
  });

  _.test("single filter", []() {
    expect(
      name_filter_set{std::regex("test$")}(
        {{"suite", "subsuite"}, "test", 1}, {}
      ),
      equal_filter_result({test_action::run, ""})
    );
    expect(
      name_filter_set{std::regex(R"(\bsubsuite\b)")}(
        {{"suite", "subsuite"}, "test", 1}, {}
      ),
      equal_filter_result({test_action::run, ""})
    );
    expect(
      name_filter_set{std::regex("mismatch")}(
        {{"suite", "subsuite"}, "test", 1}, {}
      ),
      equal_filter_result({test_action::hide, ""})
    );
  });

  _.test("multiple filters", []() {
    // hide + hide => hide
    expect(
      name_filter_set{std::regex("mismatch"), std::regex("bad")}(
        {{"suite", "subsuite"}, "test", 1}, {}
      ),
      equal_filter_result({test_action::hide, ""})
    );

    // run + run => run
    expect(
      name_filter_set{std::regex("test"), std::regex("subsuite")}(
        {{"suite", "subsuite"}, "test", 1}, {}
      ),
      equal_filter_result({test_action::run, ""})
    );

    // hide + run => run
    expect(
      name_filter_set{std::regex("mismatch"), std::regex("test")}(
        {{"suite", "subsuite"}, "test", 1}, {}
      ),
      equal_filter_result({test_action::run, ""})
    );
  });
});

struct attr_filter_fixture {
  attr_filter_fixture(bool negated, attr_filter_item filter) :
    negated(negated), filter(std::move(filter)) {}

  bool negated;
  attr_filter_item filter;
};

auto attr_name_filter_suite(bool negated) {
  auto filter = has_attr("attribute");
  std::string suite_name("has_attr(name)");
  if(negated) {
    filter = !std::move(filter);
    suite_name = "!" + suite_name;
  }

  return make_subsuite<std::tuple<>, attr_filter_fixture>(
    suite_name, bind_factory(negated, std::move(filter)), [](auto &_) {
      _.test("basic", [](auto &fixture) {
        expect(fixture.filter.attribute, equal_to("attribute"));
        expect(fixture.filter.func(nullptr), equal_to(fixture.negated));
      });

      _.test("bool_attr", [](auto &fixture) {
        bool_attr attr("attribute");

        attr_instance a1 = attr;
        expect(fixture.filter.func(&a1), equal_to(!fixture.negated));

        attr_instance a2 = attr("value");
        expect(fixture.filter.func(&a2), equal_to(!fixture.negated));
      });

      _.test("string_attr", [](auto &fixture) {
        string_attr attr("attribute");

        attr_instance a = attr("value");
        expect(fixture.filter.func(&a), equal_to(!fixture.negated));
      });

      _.test("list_attr", [](auto &fixture) {
        list_attr attr("attribute");

        attr_instance a = attr("value");
        expect(fixture.filter.func(&a), equal_to(!fixture.negated));
      });
  });
}

auto attr_value_filter_suite(bool negated) {
  auto filter = has_attr("attribute", "value");
  std::string suite_name("has_attr(name, value)");
  if(negated) {
    filter = !std::move(filter);
    suite_name = "!" + suite_name;
  }

  return make_subsuite<std::tuple<>, attr_filter_fixture>(
    suite_name, bind_factory(negated, std::move(filter)), [](auto &_) {
      _.test("basic", [](auto &fixture) {
        expect(fixture.filter.attribute, equal_to("attribute"));
        expect(fixture.filter.func(nullptr), equal_to(fixture.negated));
      });

      _.test("bool_attr", [](auto &fixture) {
        bool_attr attr("attribute");

        attr_instance a1 = attr;
        expect(fixture.filter.func(&a1), equal_to(fixture.negated));

        attr_instance a2 = attr("value");
        expect(fixture.filter.func(&a2), equal_to(!fixture.negated));
        attr_instance a3 = attr("other");
        expect(fixture.filter.func(&a3), equal_to(fixture.negated));
      });

      _.test("string_attr", [](auto &fixture) {
        string_attr attr("attribute");

        attr_instance a1 = attr("value");
        expect(fixture.filter.func(&a1), equal_to(!fixture.negated));
        attr_instance a2 = attr("other");
        expect(fixture.filter.func(&a2), equal_to(fixture.negated));
      });

      _.test("list_attr", [](auto &fixture) {
        list_attr attr("attribute");

        attr_instance a1 = attr("value");
        expect(fixture.filter.func(&a1), equal_to(!fixture.negated));
        attr_instance a2 = attr("other");
        expect(fixture.filter.func(&a2), equal_to(fixture.negated));
        attr_instance a3 = attr("other", "value");
        expect(fixture.filter.func(&a3), equal_to(!fixture.negated));
      });
  });
}

suite<> test_attr_filters("attribute filters", [](auto &_) {
  subsuite<>(_, "attr_filter_item", [](auto &_) {
    for(bool negated : {false, true}) {
      _.subsuite(attr_name_filter_suite(negated));
      _.subsuite(attr_value_filter_suite(negated));
    }
  });

  subsuite<>(_, "attr_filter", [](auto &_) {
    subsuite<>(_, "matching filters", [](auto &_) {
      _.test("empty", []() {
        bool_attr attr("bool");
        expect(
          attr_filter{}(test_name(), {}),
          equal_filter_result({test_action::run, ""})
        );
        expect(
          attr_filter{}(test_name(), {attr}),
          equal_filter_result({test_action::run, ""})
        );
      });

      _.test("has_attr(name)", []() {
        bool_attr attr1("first");
        bool_attr attr2("second");

        expect(
          attr_filter{ has_attr("first") }(test_name(), {attr1}),
          equal_filter_result({test_action::run, ""})
        );
        expect(
          attr_filter{ has_attr("first") }(test_name(), {attr1, attr2}),
          equal_filter_result({test_action::run, ""})
        );
        expect(
          attr_filter{ has_attr("first"), has_attr("second") }(
            test_name(), { attr1, attr2 }
          ),
          equal_filter_result({test_action::run, ""})
        );
      });

      _.test("has_attr(name, value)", []() {
        string_attr attr1("first");
        string_attr attr2("second");

        expect(
          attr_filter{ has_attr("first", "1") }(test_name(), {attr1("1")}),
          equal_filter_result({test_action::run, ""})
        );
        expect(
          attr_filter{ has_attr("first", "1") }(
            test_name(), {attr1("1"), attr2("2")}
          ),
          equal_filter_result({test_action::run, ""})
        );
        expect(
          attr_filter{ has_attr("first", "1"), has_attr("second", "2") }(
            test_name(), { attr1("1"), attr2("2") }
          ),
          equal_filter_result({test_action::run, ""})
        );
      });

      _.test("!has_attr(name)", []() {
        bool_attr attr("bool");

        expect(
          attr_filter{ !has_attr("mismatch") }(test_name(), {}),
          equal_filter_result({test_action::run, ""})
        );
        expect(
          attr_filter{ !has_attr("mismatch") }(test_name(), {attr}),
          equal_filter_result({test_action::run, ""})
        );
        expect(
          attr_filter{ !has_attr("mismatch"), has_attr("bool") }(
            test_name(), {attr}
          ),
          equal_filter_result({test_action::run, ""})
        );
      });

      _.test("!has_attr(name, value)", []() {
        string_attr attr("string");

        expect(
          attr_filter{ !has_attr("mismatch", "value") }(test_name(), {}),
          equal_filter_result({test_action::run, ""})
        );
        expect(
          attr_filter{ !has_attr("mismatch", "value") }(
            test_name(), {attr("value")}
          ),
          equal_filter_result({test_action::run, ""})
        );
        expect(
          attr_filter{ !has_attr("string", "mismatch") }(
            test_name(), {attr("value")}
          ),
          equal_filter_result({test_action::run, ""})
        );
      });

      _.test("skipped attr, explicit", []() {
        bool_attr attr1("first", test_action::skip);
        bool_attr attr2("second", test_action::skip);

        expect(
          attr_filter{ has_attr("first") }(test_name(), {attr1}),
          equal_filter_result({test_action::run, ""})
        );
        expect(
          attr_filter{ has_attr("first"), has_attr("second") }(
            test_name(), {attr1, attr2}
          ),
          equal_filter_result({test_action::run, ""})
        );
      });

      _.test("skipped attr, implicit", []() {
        bool_attr attr1("first", test_action::skip);
        bool_attr attr2("second");
        bool_attr attr3("third");

        attributes attrs = { attr1("message"), attr2, attr3 };
        expect(
          attr_filter{}(test_name(), attrs),
          equal_filter_result({test_action::skip, "message"})
        );
        expect(
          attr_filter{ has_attr("second") }(test_name(), attrs),
          equal_filter_result({test_action::skip, "message"})
        );
        expect(
          attr_filter{ has_attr("second"), has_attr("third") }(
            test_name(), attrs
          ),
          equal_filter_result({test_action::skip, "message"})
        );
      });
    });

    subsuite<>(_, "non-matching filters", [](auto &_) {
      _.test("has_attr(name)", []() {
        bool_attr attr("bool");

        expect(
          attr_filter{ has_attr("mismatch") }(test_name(), {}),
          equal_filter_result({test_action::hide, ""})
        );
        expect(
          attr_filter{ has_attr("mismatch") }(test_name(), {attr}),
          equal_filter_result({test_action::hide, ""})
        );
        expect(
          attr_filter{ has_attr("mismatch"), has_attr("bool") }(
            test_name(), {attr}
          ),
          equal_filter_result({test_action::hide, ""})
        );
      });

      _.test("has_attr(name, value)", []() {
        string_attr attr("string");

        expect(
          attr_filter{ has_attr("mismatch", "value") }(test_name(), {}),
          equal_filter_result({test_action::hide, ""})
        );
        expect(
          attr_filter{ has_attr("mismatch", "value") }(
            test_name(), {attr("value")}
          ),
          equal_filter_result({test_action::hide, ""})
        );
        expect(
          attr_filter{ has_attr("string", "mismatch") }(
            test_name(), {attr("value")}
          ),
          equal_filter_result({test_action::hide, "value"})
        );
      });

      _.test("!has_attr(name)", []() {
        bool_attr attr1("first");
        bool_attr attr2("second");

        attributes first_attr = { attr1("1") };
        attributes both_attrs = { attr1("1"), attr2("2") };

        expect(
          attr_filter{ !has_attr("first") }(test_name(), first_attr),
          equal_filter_result({test_action::hide, "1"})
        );
        expect(
          attr_filter{ !has_attr("first") }(test_name(), both_attrs),
          equal_filter_result({test_action::hide, "1"})
        );
        expect(
          attr_filter{ !has_attr("first"), !has_attr("second") }(
            test_name(), both_attrs
          ),
          equal_filter_result({test_action::hide, "1"})
        );
        expect(
          attr_filter{ !has_attr("first"), has_attr("second") }(
            test_name(), both_attrs
          ),
          equal_filter_result({test_action::hide, "1"})
        );
        expect(
          attr_filter{ has_attr("first"), !has_attr("second") }(
            test_name(), both_attrs
          ),
          equal_filter_result({test_action::hide, "2"})
        );
      });

      _.test("!has_attr(name, value)", []() {
        string_attr attr1("first");
        string_attr attr2("second");

        attributes first_attr = { attr1("1") };
        attributes both_attrs = { attr1("1"), attr2("2") };

        expect(
          attr_filter{ !has_attr("first", "1") }(test_name(), first_attr),
          equal_filter_result({test_action::hide, "1"})
        );
        expect(
          attr_filter{ !has_attr("first", "1") }(test_name(), both_attrs),
          equal_filter_result({test_action::hide, "1"})
        );
        expect(
          attr_filter{ !has_attr("first", "1"), !has_attr("second", "2") }(
            test_name(), both_attrs
          ),
          equal_filter_result({test_action::hide, "1"})
        );
        expect(
          attr_filter{ !has_attr("first", "1"), has_attr("second", "2") }(
            test_name(), both_attrs
          ),
          equal_filter_result({test_action::hide, "1"})
        );
        expect(
          attr_filter{ has_attr("first", "1"), !has_attr("second", "2") }(
            test_name(), both_attrs
          ),
          equal_filter_result({test_action::hide, "2"})
        );
      });

      _.test("skipped attr", []() {
        bool_attr attr1("first", test_action::skip);
        bool_attr attr2("second", test_action::skip);

        expect(
          attr_filter{ has_attr("mismatch") }(test_name(), {attr1}),
          equal_filter_result({test_action::hide, ""})
        );
        expect(
          attr_filter{ has_attr("first"), has_attr("mismatch") }(
            test_name(), {attr1, attr2}
          ),
          equal_filter_result({test_action::hide, ""})
        );
      });
    });
  });

  subsuite<>(_, "attr_filter_set", [](auto &_) {
    _.test("empty set", []() {
      bool_attr attr1("first", test_action::skip);
      bool_attr attr2("second");

      expect(
        attr_filter_set{}(test_name(), {}),
        equal_filter_result({test_action::indeterminate, ""})
      );
      expect(
        attr_filter_set{}(test_name(), {attr2}),
        equal_filter_result({test_action::indeterminate, ""})
      );
      expect(
        attr_filter_set{}(test_name(), {attr1}),
        equal_filter_result({test_action::indeterminate, ""})
      );
    });

    _.test("single filter", []() {
      bool_attr attr1("first", test_action::skip);
      bool_attr attr2("second");

      expect(
        attr_filter_set{ {has_attr("first")} }(test_name(), {}),
        equal_filter_result({test_action::hide, ""})
      );
      expect(
        attr_filter_set{ {!has_attr("first")} }(test_name(), {}),
        equal_filter_result({test_action::run, ""})
      );
      expect(
        attr_filter_set{ {has_attr("first")} }(test_name(), {attr1}),
        equal_filter_result({test_action::run, ""})
      );
      attributes attrs = { attr1("1"), attr2("2") };
      expect(
        attr_filter_set{ {has_attr("second")} }(test_name(), attrs),
        equal_filter_result({test_action::skip, "1"})
      );
      expect(
        attr_filter_set{ {!has_attr("first")} }(test_name(), attrs),
        equal_filter_result({test_action::hide, "1"})
      );
    });

    _.test("multiple filters", []() {
      bool_attr attr1("first", test_action::skip);
      bool_attr attr2("second");

      attributes both_attrs = { attr1("1"), attr2("2") };

      // hide + hide => hide
      expect(
        attr_filter_set{ {has_attr("first")}, {has_attr("second")} }(
          test_name(), {}
        ),
        equal_filter_result({test_action::hide, ""})
      );
      expect(
        attr_filter_set{ {!has_attr("first")}, {!has_attr("second")} }(
          test_name(), both_attrs
        ),
        equal_filter_result({test_action::hide, "1"})
      );

      // run + run => run
      expect(
        attr_filter_set{ {has_attr("first")}, {has_attr("second")} }(
          test_name(), both_attrs
        ),
        equal_filter_result({test_action::run, ""})
      );

      // skip + skip => skip
      expect(
        attr_filter_set{ {has_attr("second")}, {has_attr("second")} }(
          test_name(), both_attrs
        ),
        equal_filter_result({test_action::skip, "1"})
      );

      // hide + skip => skip
      expect(
        attr_filter_set{ {has_attr("other")}, {has_attr("second")} }(
          test_name(), both_attrs
        ),
        equal_filter_result({test_action::skip, "1"})
      );

      // hide + run => run
      expect(
        attr_filter_set{ {has_attr("other")}, {has_attr("second")} }(
          test_name(), {attr2}
        ),
        equal_filter_result({test_action::run, ""})
      );

      // run + skip => run
      expect(
        attr_filter_set{ {has_attr("first")}, {has_attr("second")} }(
          test_name(), both_attrs
        ),
        equal_filter_result({test_action::run, ""})
      );
    });
  });
});

suite<> test_combined_filters("combined filters", [](auto &_) {
  _.test("no filters", []() {
    expect(
      filter_set{}(test_name(), {}),
      equal_filter_result({test_action::indeterminate, ""})
    );
  });

  _.test("name filter only", []() {
    expect(
      filter_set{ {std::regex("test$")}, {} }(
        {{"suite", "subsuite"}, "test", 1}, {}
      ),
      equal_filter_result({test_action::run, ""})
    );
    expect(
      filter_set{ {std::regex("mismatch")}, {} }(
        {{"suite", "subsuite"}, "test", 1}, {}
      ),
      equal_filter_result({test_action::hide, ""})
    );
  });

  _.test("attr filter only", []() {
    bool_attr attr1("first", test_action::skip);
    bool_attr attr2("second");

    expect(
      filter_set{ {}, {{has_attr("first")}} }(
        test_name(), {attr1}
      ),
      equal_filter_result({test_action::run, ""})
    );
    expect(
      filter_set{ {}, {{has_attr("second")}} }(
        test_name(), {attr1("message"), attr2}
      ),
      equal_filter_result({test_action::skip, "message"})
    );
    expect(
      filter_set{ {}, {{has_attr("other")}} }(
        test_name(), {attr1}
      ),
      equal_filter_result({test_action::hide, ""})
    );
  });

  _.test("name and attr filters", []() {
    bool_attr attr1("first", test_action::skip);
    bool_attr attr2("second");

    // hide + hide => hide
    expect(
      filter_set{ {std::regex("mismatch")}, {{has_attr("other")}} }(
        {{"suite", "subsuite"}, "test", 1}, {attr1}
      ),
      equal_filter_result({test_action::hide, ""})
    );

    // hide + run => hide
    expect(
      filter_set{ {std::regex("mismatch")}, {{has_attr("first")}} }(
        {{"suite", "subsuite"}, "test", 1}, {attr1}
      ),
      equal_filter_result({test_action::hide, ""})
    );

    // hide + skip => hide
    expect(
      filter_set{ {std::regex("mismatch")}, {{has_attr("second")}} }(
        {{"suite", "subsuite"}, "test", 1}, {attr1, attr2}
      ),
      equal_filter_result({test_action::hide, ""})
    );

    // run + hide => hide
    expect(
      filter_set{ {std::regex("test$")}, {{has_attr("other")}} }(
        {{"suite", "subsuite"}, "test", 1}, {attr1}
      ),
      equal_filter_result({test_action::hide, ""})
    );

    // run + skip => skip
    expect(
      filter_set{ {std::regex("test$")}, {{has_attr("second")}} }(
        {{"suite", "subsuite"}, "test", 1}, {attr1("message"), attr2}
      ),
      equal_filter_result({test_action::skip, "message"})
    );

    // run + run => run
    expect(
      filter_set{ {std::regex("test$")}, {{has_attr("first")}} }(
        {{"suite", "subsuite"}, "test", 1}, {attr1}
      ),
      equal_filter_result({test_action::run, ""})
    );
  });
});
