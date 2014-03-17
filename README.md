# mettle

`mettle` is a (work-in-progress) library for writing unit tests using modern
C++ (read: C++11 and maybe some C++14). Its main goals are readability and
avoidance of unnecessary macros.

## A Brief Example

A picture is worth a thousand words, and code is probably worth an appreciable
amount of words (let's say 100), so let's take a look:

```c++
#include "mettle.hpp"

using namespace mettle;

struct basic_data {
  int foo;
};

suite<basic_data> basic("basic suite", [](suite<basic_data> &_) {
  _.setup([](basic_data &) {
  });

  _.teardown([](basic_data &) {
  });

  _.test("a test", [](basic_data &) {
    expect(true, equals(true));
  });

  for(int i = 0; i < 4; i++) {
    _.test("test number " + std::to_string(i), [i](basic_data &) {
      expect(i % 2, equals(0));
    });
  }
});
```
