#include <mettle.hpp>
using namespace mettle;

#include "../helpers.hpp"
#include "../../src/test_file.hpp"

suite<> test_test_file("test_file", [](auto &_) {
  subsuite<>(_, "file expansion", [](auto &_) {
    _.test("single word", []() {
      expect(test_file("filename").args(), array("filename"));
    });

    _.test("multiple words", []() {
      expect(test_file("file arg1 arg2").args(), array("file", "arg1", "arg2"));
      expect(test_file("file\targ1\targ2").args(),
             array("file", "arg1", "arg2"));
    });

    _.test("quoting", []() {
      expect(test_file("\"two words\"").args(), array("two words"));
      expect(test_file("'two words'").args(), array("two words"));
    });

    _.test("escaping", []() {
      expect(test_file("arg\\ 1 arg\\ 2").args(), array("arg 1", "arg 2"));
      expect(test_file("o\\'toole").args(), array("o'toole"));
      expect(test_file("\\\"quote\\\"").args(), array("\"quote\""));
    });

    _.test("invalid syntax", []() {
      expect([]() { test_file("glob["); },
             thrown<std::runtime_error>("invalid glob \"glob[\""));
    });
  });

  _.test("program_options validate()", []() {
    boost::any value;
    validate(value, {"arg1 arg2"}, static_cast<test_file*>(nullptr), 0);
    expect(value, match_any<test_file>(
      filter([](auto &&x) { return x.args(); }, array("arg1", "arg2"))
    ));

    expect(
      []() {
        boost::any v;
        validate(v, {"glob["}, static_cast<test_file*>(nullptr), 0);
      },
      thrown<std::exception>("the argument ('glob[') for option is invalid")
    );
  });
});
