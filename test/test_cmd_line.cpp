#include <mettle.hpp>
using namespace mettle;

#include <mettle/driver/cmd_line.hpp>
#include <boost/program_options.hpp>
#include "helpers.hpp"

auto match_filter_item(attr_instance attr, bool matched) {
  std::ostringstream ss;
  ss << "filter_item(" << to_printable(attr.attribute.name()) << ") => "
     << to_printable(matched);
  return make_matcher(
    [attr = std::move(attr), matched](const attr_filter_item &actual) {
      return actual.attribute == attr.attribute.name() &&
             actual.func(&attr) == matched;
    }, ss.str()
  );
}

auto equal_option_description(std::string name) {
  using boost::program_options::option_description;
  return make_matcher(
    std::move(name),
    [](const option_description *actual, const std::string &name) {
      return actual && actual->long_name() == name;
    }, {"option_description(", ")"}
  );
}

suite<> test_parse_attr("parse_attr()", [](auto &_) {
  _.test("attr", []() {
    bool_attr attr("attr");

    auto filter = parse_attr("attr");
    expect(filter.size(), equal_to(1u));
    expect(filter, array( match_filter_item(attr, true) ));
    expect(filter(test_name(), {attr}),
           equal_filter_result( {test_action::run, ""} ));
  });

  _.test("attr=value", []() {
    string_attr attr("attr");

    auto filter = parse_attr("attr=value");
    expect(filter.size(), equal_to(1u));
    expect(filter, array( match_filter_item(attr("value"), true) ));
    expect(filter(test_name(), {attr("value")}),
           equal_filter_result( {test_action::run, ""} ));
  });

  _.test("attr=", []() {
    string_attr attr("attr");

    auto filter = parse_attr("attr=");
    expect(filter.size(), equal_to(1u));
    expect(filter, array( match_filter_item(attr(""), true) ));
    expect(filter(test_name(), {attr("")}),
           equal_filter_result( {test_action::run, ""} ));
  });

  _.test("!attr", []() {
    bool_attr attr("attr");

    auto filter = parse_attr("!attr");
    expect(filter.size(), equal_to(1u));
    expect(filter, array( match_filter_item(attr, false) ));
    attributes attrs = {attr("message")};
    expect(filter(test_name(), attrs),
           equal_filter_result( {test_action::hide, "message"} ));
  });

  _.test("!attr=value", []() {
    string_attr attr("attr");

    auto filter = parse_attr("!attr=value");
    expect(filter.size(), equal_to(1u));
    expect(filter, array( match_filter_item(attr("value"), false) ));
    attributes attrs = {attr("value")};
    expect(filter(test_name(), attrs),
           equal_filter_result( {test_action::hide, "value"} ));
  });

  _.test("!attr=", []() {
    string_attr attr("attr");

    auto filter = parse_attr("!attr=");
    expect(filter.size(), equal_to(1u));
    expect(filter, array( match_filter_item(attr(""), false) ));
    attributes attrs = {attr("")};
    expect(filter(test_name(), attrs),
           equal_filter_result( {test_action::hide, ""} ));
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
           equal_filter_result( {test_action::run, ""} ));
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
           equal_filter_result( {test_action::run, ""} ));
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
           equal_filter_result( {test_action::hide, ""} ));
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
           equal_filter_result( {test_action::hide, "2"} ));
  });

  _.test("parse failures", []() {
    expect([]() { parse_attr(""); },
           thrown<std::invalid_argument>("unexpected end of string"));
    expect([]() { parse_attr("!"); },
           thrown<std::invalid_argument>("unexpected end of string"));
    expect([]() { parse_attr(","); },
           thrown<std::invalid_argument>("expected attribute name"));
    expect([]() { parse_attr("="); },
           thrown<std::invalid_argument>("expected attribute name"));
    expect([]() { parse_attr("attr,"); },
           thrown<std::invalid_argument>("unexpected end of string"));
    expect([]() { parse_attr("attr,,"); },
           thrown<std::invalid_argument>("expected attribute name"));
    expect([]() { parse_attr("attr,="); },
           thrown<std::invalid_argument>("expected attribute name"));
  });
});

suite<> test_program_options("program_options utilities", [](auto &_) {
  namespace opts = boost::program_options;

  subsuite<opts::options_description>(_, "options_description utilities",
                                      [](auto &_) {
    _.setup([](opts::options_description &desc) {
      desc.add_options()
        ("foo,f", "foo option")
        ("bar,b", "bar option")
      ;
    });

    subsuite<>(_, "has_option()", [](auto &_) {
      _.test("map has option", [](opts::options_description &desc) {
        opts::variables_map map;
        map.emplace(std::pair<std::string, opts::variable_value>("foo", {}));

        expect(has_option(desc, map), equal_option_description("foo"));
      });

      _.test("map doesn't have option", [](opts::options_description &desc) {
        opts::variables_map map;

        expect(has_option(desc, map), equal_to(nullptr));
      });
    });

    subsuite<>(_, "filter_options()", [](auto &_) {
      _.test("all matching", [](opts::options_description &desc) {
        std::vector<std::string> args = {"--foo", "--bar"};
        auto parsed = opts::command_line_parser(args)
          .options(desc).allow_unregistered().run();

        expect(filter_options(parsed, desc), array("--foo", "--bar"));
      });

      _.test("none matching", [](opts::options_description &desc) {
        std::vector<std::string> args = {"--goat", "--cow"};
        auto parsed = opts::command_line_parser(args)
          .options(desc).allow_unregistered().run();

        expect(filter_options(parsed, desc), array());
      });

      _.test("some matching", [](opts::options_description &desc) {
        std::vector<std::string> args = {"--foo", "--goat"};
        auto parsed = opts::command_line_parser(args)
          .options(desc).allow_unregistered().run();

        expect(filter_options(parsed, desc), array("--foo"));
      });
    });
  });

  subsuite<>(_, "validate()", [](auto &_) {
    _.test("color_option", []() {
      boost::any value;
      validate(value, {"always"}, static_cast<color_option*>(nullptr), 0);
      expect(value, any_equal(color_option::always));

      value.clear();
      validate(value, {"never"}, static_cast<color_option*>(nullptr), 0);
      expect(value, any_equal(color_option::never));

      value.clear();
      validate(value, {"auto"}, static_cast<color_option*>(nullptr), 0);
      expect(value, any_equal(color_option::automatic));

      expect(
        []() {
          boost::any value;
          validate(value, {"invalid"}, static_cast<color_option*>(nullptr), 0);
        },
        thrown<std::exception>("the argument ('invalid') for option is invalid")
      );
    });

    _.test("attr_filter_set", []() {
      string_attr attr("attr");

      boost::any value;
      validate(value, {"attr=val"}, static_cast<attr_filter_set*>(nullptr), 0);
      auto filter = boost::any_cast<attr_filter_set>(value);
      expect(filter, array(
        array( match_filter_item(attr("val"), true) )
      ));

      expect(
        []() {
          boost::any value;
          validate(value, {"!"}, static_cast<attr_filter_set*>(nullptr), 0);
        },
        thrown<std::exception>("the argument ('!') for option is invalid")
      );
    });

    _.test("name_filter_set", []() {
      boost::any value;
      validate(value, {"name"}, static_cast<name_filter_set*>(nullptr), 0);
      auto filter = boost::any_cast<name_filter_set>(value);
      expect(
        filter({{"suite", "subsuite"}, "name", 1}, {}),
        equal_filter_result({test_action::run, ""})
      );
      expect(
        filter({{"suite", "subsuite"}, "mismatch", 1}, {}),
        equal_filter_result({test_action::hide, ""})
      );

      expect(
        []() {
          boost::any value;
          validate(value, {"["}, static_cast<name_filter_set*>(nullptr), 0);
        },
        thrown<std::exception>("the argument ('[') for option is invalid")
      );
    });

    _.test("std::chrono::milliseconds", []() {
      using ms = std::chrono::milliseconds;
      boost::any value;
      validate(value, {"1000"}, static_cast<ms*>(nullptr), 0);
      expect(value, any_equal(ms(1000)));

      expect(
        []() {
          boost::any value;
          validate(value, {"invalid"}, static_cast<ms*>(nullptr), 0);
        },
        thrown<std::exception>("the argument ('invalid') for option is invalid")
      );
    });

    _.test("std::experimental::optional", []() {
      using METTLE_OPTIONAL_NS::optional;
      boost::any value;
      validate(value, {"1"}, static_cast<optional<int>*>(nullptr), 0);
      expect(value, any_equal(optional<int>(1)));

      expect(
        []() {
          boost::any value;
          validate(value, {"invalid"}, static_cast<optional<int>*>(nullptr), 0);
        },
        thrown<std::exception>("the argument ('invalid') for option is invalid")
      );
    });
  });

});
