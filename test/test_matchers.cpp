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
  });

  _.test("equal_to()", []() {
    expect(true, equal_to(true));
    expect(123, equal_to(123));
  });

  _.test("not_equal_to()", []() {
    expect(true, not_equal_to(false));
    expect(123, not_equal_to(1234));
  });

  _.test("greater()", []() {
    expect(123, greater(0));
  });

  _.test("greater_equal()", []() {
    expect(123, greater_equal(0));
  });

  _.test("less()", []() {
    expect(123, less(1000));
  });

  _.test("less_equal()", []() {
    expect(123, less_equal(1000));
  });

  _.test("is_not()", []() {
    expect(123, is_not(equal_to(100)));
    expect(123, is_not(100));
  });

  _.test("any_of()", []() {
    expect(123, any_of(equal_to(1), equal_to(2), equal_to(123)));
    expect(123, any_of(1, 2, 123));
    expect(123, is_not(any_of()));
  });

  _.test("all_of()", []() {
    expect(123, all_of(not_equal_to(1), not_equal_to(2), greater(3)));
    expect(123, all_of());
  });

  _.test("has_element()", []() {
    expect(std::vector<int>{1, 2, 3}, has_element(1));
    expect(std::vector<int>{1, 2, 3}, is_not(has_element(4)));
  });

  _.test("array()", []() {
    expect(std::vector<int>{}, array());
    expect(std::vector<int>{1, 2, 3}, array(1, 2, 3));
  });

});
