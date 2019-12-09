#include <mettle.hpp>
using namespace mettle;

#include "../helpers.hpp"
#include "../../src/mettle/test_command.hpp"

suite<> test_test_file("test_command", [](auto &_) {
  subsuite<>(_, "file expansion", [](auto &_) {
    _.test("single word", []() {
      expect(test_command("filename").args(), array("filename"));
    });

    _.test("multiple words", []() {
      expect(test_command("file arg1 arg2").args(),
             array("file", "arg1", "arg2"));
      expect(test_command("file\targ1\targ2").args(),
             array("file", "arg1", "arg2"));
    });

    _.test("quoting", []() {
      expect(test_command("\"two words\"").args(), array("two words"));
#ifndef _WIN32
      expect(test_command("'two words'").args(), array("two words"));
#endif
    });

    _.test("escaping", []() {
#ifndef _WIN32
      expect(test_command("arg\\ 1 arg\\ 2").args(), array("arg 1", "arg 2"));
      expect(test_command("o\\'toole").args(), array("o'toole"));
#endif
      expect(test_command("\\\"quote\\\"").args(), array("\"quote\""));
    });

    _.test("invalid syntax", []() {
      expect([]() { test_command("glob["); },
             thrown<std::runtime_error>("no matches found: glob["));
    });
  });

  _.test("program_options validate()", []() {
    boost::any value;
    validate(value, {"arg1 arg2"}, static_cast<test_command*>(nullptr), 0);
    expect(value, match_any<test_command>(
      filter([](auto &&x) { return x.args(); }, array("arg1", "arg2"))
    ));

    expect(
      []() {
        boost::any v;
        validate(v, {"glob["}, static_cast<test_command*>(nullptr), 0);
      },
      thrown<std::exception>("the argument ('glob[') for option is invalid")
    );
  });
});
