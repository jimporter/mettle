#include "mettle.hpp"
using namespace mettle;

#include <vector>
#include <list>

template<typename T>
std::string stringify(T &&t) {
  std::stringstream s;
  s << ensure_printable(std::forward<T>(t));
  return s.str();
}

suite<> output("test debug printing", [](auto &_){
  _.test("test printing primitives", [](){
    expect(stringify(666), equal_to("666"));
    expect(stringify(2.5), equal_to("2.5"));
    expect(stringify(true), equal_to("true"));
    expect(stringify(false), equal_to("false"));
    expect(stringify("text"), equal_to("text"));
    expect(stringify(nullptr), equal_to("nullptr"));
  });

  _.test("test printing iterables", [](){
    expect(stringify(std::vector<int>{}), equal_to("[]"));
    expect(stringify(std::vector<int>{1}), equal_to("[1]"));
    expect(stringify(std::vector<int>{1, 2, 3}), equal_to("[1, 2, 3]"));
    expect(stringify(std::list<int>{1, 2, 3}), equal_to("[1, 2, 3]"));

    // No test for zero-sized arrays because those are illegal in C++ (even
    // though many compilers accept them).
    int arr1[] = {1};
    expect(stringify(arr1), equal_to("[1]"));
    int arr2[] = {1, 2, 3};
    expect(stringify(arr2), equal_to("[1, 2, 3]"));
  });

  _.test("test printing custom types", []() {
    struct some_type {};

    // This will get a string from type_info, which could be anything...
    expect(stringify(some_type{}), anything());
  });
});
