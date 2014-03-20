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

    int arr1[] = {1};
    expect(ensure_printable(arr1), equal_to("[1]"));
    int arr2[] = {1, 2, 3};
    expect(ensure_printable(arr2), equal_to("[1, 2, 3]"));
  });
});
