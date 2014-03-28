#include <stdexcept>
#include <vector>

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

  _.test("member()", []() {
    expect(std::vector<int>{}, is_not(member(0)));
    expect(std::vector<int>{1, 2, 3}, member(1));
    expect(std::vector<int>{1, 2, 3}, is_not(member(4)));

    expect(member(123).desc(), equal_to("member 123"));
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

  _.test("thrown()", []() {
    auto thrower = []() { throw std::runtime_error("message"); };
    expect(thrower, thrown<std::runtime_error>());
    expect(thrower, thrown<std::exception>());
    expect(thrower, is_not(thrown<std::logic_error>()));
    expect(thrower, thrown());
    expect(thrower, thrown<std::runtime_error>("message"));
    expect(thrower, thrown<std::runtime_error>(is_not("wrong")));
    expect(thrower, is_not(thrown<std::logic_error>("message")));
    expect(thrower, is_not(thrown<std::logic_error>( is_not("wrong") )));

    auto int_thrower = []() { throw 123; };
    expect(int_thrower, thrown<int>());
    expect(int_thrower, is_not(thrown<std::exception>()));
    expect(int_thrower, thrown());
    expect(int_thrower, thrown_raw<int>(123));
    expect(int_thrower, is_not(thrown_raw<int>(0)));
    expect(int_thrower, is_not(thrown<std::exception>("message")));
    expect(int_thrower, is_not(thrown<std::exception>( is_not("wrong") )));

    auto noop = []() {};
    expect(noop, is_not(thrown<std::exception>()));
    expect(noop, is_not(thrown()));
    expect(noop, is_not(thrown<std::exception>("message")));
    expect(noop, is_not(thrown<std::exception>( is_not("wrong") )));
  });

});
