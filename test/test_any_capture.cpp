#include <mettle.hpp>
using namespace mettle;

#include "copy_counter.hpp"

void func() {}

suite<> test_capture("any_capture", [](auto &_) {
  using detail::any_capture;
  using detail::unwrap_capture;

  _.test("capture int", []() {
    any_capture<int> capture1(1);
    expect(unwrap_capture(capture1), equal_to(1));

    int i = 2;
    any_capture<int> capture2(i);
    expect(unwrap_capture(capture2), equal_to(i));
  });

  _.test("capture std::string", []() {
    any_capture<std::string> capture1("foo");
    expect(unwrap_capture(capture1), equal_to("foo"));

    std::string s = "bar";
    any_capture<std::string> capture2(s);
    expect(unwrap_capture(capture2), equal_to(s));
  });

  _.test("capture char[]", []() {
    char s[] = "foo";
    any_capture<char[4]> capture(s);
    expect(unwrap_capture(capture), equal_to(std::string("foo")));
  });

  _.test("capture function", []() {
    any_capture<void()> capture(func);

    expect(unwrap_capture(capture), equal_to(func));
    expect(unwrap_capture(capture), equal_to(&func));
  });

  _.test("capture by copy", []() {
    copyable_type t;
    any_capture<copyable_type> capture(t);

    expect("number of copies", unwrap_capture(capture).copies, equal_to(1));
  });

  _.test("capture by move", []() {
    moveable_type t;
    any_capture<moveable_type> capture(std::move(t));

    expect("number of copies", unwrap_capture(capture).copies, equal_to(0));
    expect("number of moves", unwrap_capture(capture).moves, equal_to(1));
  });

  _.test("capture array by copy", []() {
    copyable_type t[2];
    any_capture<copyable_type[2]> capture(t);

    expect("number of copies", unwrap_capture(capture), each(
      filter([](auto &&x) { return x.copies; }, equal_to(1))
    ));
  });

  _.test("capture array by move", []() {
    moveable_type t[2];
    any_capture<moveable_type[2]> capture(std::move(t));

    expect("number of copies", unwrap_capture(capture), each(
      filter([](auto &&x) { return x.copies; }, equal_to(0))
    ));
    expect("number of moves", unwrap_capture(capture), each(
      filter([](auto &&x) { return x.moves; }, equal_to(1))
    ));
  });
});
