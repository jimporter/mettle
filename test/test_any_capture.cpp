#include <mettle.hpp>
using namespace mettle;

#include "copy_counter.hpp"

void func() {}

suite<> test_capture("any_capture", [](auto &_) {
  _.test("capture int", []() {
    detail::any_capture<int> capture1(1);
    expect(capture1.value, equal_to(1));

    int i = 2;
    detail::any_capture<int> capture2(i);
    expect(capture2.value, equal_to(i));
  });

  _.test("capture std::string", []() {
    detail::any_capture<std::string> capture1("foo");
    expect(capture1.value, equal_to("foo"));

    std::string s = "bar";
    detail::any_capture<std::string> capture2(s);
    expect(capture2.value, equal_to(s));
  });

  _.test("capture char[]", []() {
    char s[] = "foo";
    detail::any_capture<char[4]> capture(s);
    expect(capture.value, equal_to(std::string("foo")));
  });

  _.test("capture function", []() {
    detail::any_capture<void()> capture(func);

    expect(capture.value, equal_to(func));
    expect(capture.value, equal_to(&func));
  });

  _.test("capture by copy", []() {
    copyable_type t;
    detail::any_capture<copyable_type> capture(t);

    expect("number of copies", capture.value.copies, equal_to(1));
  });

  _.test("capture by move", []() {
    moveable_type t;
    detail::any_capture<moveable_type> capture(std::move(t));

    expect("number of copies", capture.value.copies, equal_to(0));
    expect("number of moves", capture.value.moves, equal_to(1));
  });

  _.test("capture array by copy", []() {
    copyable_type t[2];
    detail::any_capture<copyable_type[2]> capture(t);

    expect("number of copies", capture.value, each(
      filter([](auto &&x) { return x.copies; }, equal_to(1))
    ));
  });

  attributes move_arr_attrs;
#if defined(_MSC_VER) && _MSC_VER < 1920
  move_arr_attrs.insert(skip("capture array by move fails on MSVC 2017"));
#endif

  _.test("capture array by move", move_arr_attrs, []() {
    moveable_type t[2];
    detail::any_capture<moveable_type[2]> capture(std::move(t));

    expect("number of copies", capture.value, each(
      filter([](auto &&x) { return x.copies; }, equal_to(0))
    ));
    expect("number of moves", capture.value, each(
      filter([](auto &&x) { return x.moves; }, equal_to(1))
    ));
  });
});
