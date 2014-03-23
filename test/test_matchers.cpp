#include "mettle.hpp"
using namespace mettle;

struct some_type {
  some_type(const some_type &) = delete;
  some_type & operator =(const some_type &) = delete;
};

suite<> matcher_tests("test matchers", [](auto &_) {

  _.test("anything()", []() {
    expect(true, anything());
    expect(false, anything());
    expect(123, anything());
    expect(some_type{}, anything());

    expect(anything().desc(), equal_to("anything"));
  });

  _.test("equal_to()", []() {
    expect(true, equal_to(true));
    expect(123, equal_to(123));

    expect(equal_to(123).desc(), equal_to("123"));
  });

  _.test("not_equal_to()", []() {
    expect(true, not_equal_to(false));
    expect(123, not_equal_to(1234));

    expect(not_equal_to(123).desc(), equal_to("not 123"));
  });

  _.test("greater()", []() {
    expect(123, greater(0));

    expect(greater(123).desc(), equal_to("> 123"));
  });

  _.test("greater_equal()", []() {
    expect(123, greater_equal(0));

    expect(greater_equal(123).desc(), equal_to(">= 123"));
  });

  _.test("less()", []() {
    expect(123, less(1000));

    expect(less(123).desc(), equal_to("< 123"));
  });

  _.test("less_equal()", []() {
    expect(123, less_equal(1000));

    expect(less_equal(123).desc(), equal_to("<= 123"));
  });

  _.test("is_not()", []() {
    expect(123, is_not(equal_to(100)));
    expect(123, is_not(100));

    expect(is_not(123).desc(), equal_to("not 123"));
  });

  _.test("any_of()", []() {
    expect(123, any_of(equal_to(1), equal_to(2), equal_to(123)));
    expect(123, any_of(1, 2, 123));
    expect(123, is_not(any_of(1, 2, 3)));
    expect(123, is_not(any_of()));

    expect(any_of(1, 2, 3).desc(), equal_to("any of(1, 2, 3)"));
  });

  _.test("all_of()", []() {
    expect(123, all_of(123));
    expect(123, all_of(not_equal_to(1), not_equal_to(2), greater(3)));
    expect(123, all_of());

    expect(all_of(1, 2, 3).desc(), equal_to("all of(1, 2, 3)"));
  });

  _.test("has_element()", []() {
    expect(std::vector<int>{}, is_not(has_element(0)));
    expect(std::vector<int>{1, 2, 3}, has_element(1));
    expect(std::vector<int>{1, 2, 3}, is_not(has_element(4)));

    expect(has_element(123).desc(), equal_to("has 123"));
  });

  _.test("each()", []() {
    expect(std::vector<int>{}, each( is_not(anything()) ));
    expect(std::vector<int>{1, 2, 3}, each( greater(0)) );
    expect(std::vector<int>{1, 2, 3}, is_not( each(less(2)) ));

    expect(each(123).desc(), equal_to("each 123"));
  });

  _.test("array()", []() {
    expect(std::vector<int>{}, array());
    expect(std::vector<int>{1, 2, 3}, array(1, 2, 3));

    expect(array(1, 2, 3).desc(), equal_to("[1, 2, 3]"));
  });

});
