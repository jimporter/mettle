#include <mettle.hpp>
using namespace mettle;

struct my_type {};

namespace my_namespace {
  struct another_type {};
}

suite<> test_type_name("type_name()", [](auto &_) {
  _.test("primitives", []() {
    expect(type_name<int>(), equal_to("int"));
    expect(type_name<int &>(), equal_to("int &"));
    expect(type_name<int &&>(), equal_to("int &&"));
    expect(type_name<const int>(), equal_to("const int"));
    expect(type_name<volatile int>(), equal_to("volatile int"));
    expect(type_name<const volatile int>(), equal_to("const volatile int"));
  });

  _.test("custom types", []() {
    expect(type_name<my_type>(), equal_to("my_type"));
    expect(type_name<my_namespace::another_type>(),
           equal_to("my_namespace::another_type"));
  });
});
