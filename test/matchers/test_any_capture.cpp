#include <mettle.hpp>
using namespace mettle;

#include "../copy_counter.hpp"

void func() {}

suite<> test_capture("any_capture", [](auto &_) {
  _.test("capture int", []() {
    any_capture<int> capture1(1);
    expect(capture1.value, equal_to(1));

    int i = 2;
    any_capture<int> capture2(i);
    expect(capture2.value, equal_to(i));
  });

  _.test("capture std::string", []() {
    any_capture<std::string> capture1("foo");
    expect(capture1.value, equal_to("foo"));

    std::string s = "bar";
    any_capture<std::string> capture2(s);
    expect(capture2.value, equal_to(s));
  });

  _.test("capture function", []() {
    any_capture<void()> capture(func);

    expect(capture.value, equal_to(func));
    expect(capture.value, equal_to(&func));
  });

  _.test("capture by copy", []() {
    copyable_type t;
    any_capture<copyable_type> capture(t);

    expect("number of copies", capture.value.copies, equal_to(1));
  });

  _.test("capture by move", []() {
    moveable_type t;
    any_capture<moveable_type> capture(std::move(t));

    expect("number of copies", capture.value.copies, equal_to(0));
    expect("number of moves", capture.value.moves, equal_to(1));
  });

  _.test("capture array by copy", []() {
    copyable_type t[2];
    any_capture<copyable_type[2]> capture(t);

    expect("number of copies", capture.value, each(
      filter([](auto &&x) { return x.copies; }, equal_to(1))
    ));
  });

// MSVC doesn't support moving arrays, despite it being legal (I think).
#if !defined(_MSC_VER) || defined(__clang__)
  _.test("capture array by move", []() {
    moveable_type t[2];
    any_capture<moveable_type[2]> capture(std::move(t));

    expect("number of copies", capture.value, each(
      filter([](auto &&x) { return x.copies; }, equal_to(0))
    ));
    expect("number of moves", capture.value, each(
      filter([](auto &&x) { return x.moves; }, equal_to(1))
    ));
  });
#endif
});
