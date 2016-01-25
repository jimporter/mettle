#include <mettle.hpp>
using namespace mettle;

struct my_type {};

namespace my_namespace {
  struct another_type {};
}

suite<> test_type_name("type_name()", [](auto &_) {
  _.test("primitives", []() {
    expect(type_name<int>(), equal_to("int"));
    expect(type_name<int &>(), regex_match("int\\s*&"));
    expect(type_name<int &&>(), regex_match("int\\s*&&"));
    expect(type_name<const int>(), equal_to("const int"));
    expect(type_name<volatile int>(), equal_to("volatile int"));
    expect(type_name<const volatile int>(), any(
      equal_to("const volatile int"),
      equal_to("volatile const int")
    ));
  });

  _.test("custom types", []() {
    expect(type_name<my_type>(), regex_match("(struct )?my_type$"));
    expect(type_name<my_namespace::another_type>(),
           regex_match("(struct )?my_namespace::another_type$"));
  });
});
