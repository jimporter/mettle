# mettle
---

**mettle** is a library for writing unit tests using modern C++ (read: C++14).
Its main goals are readability and avoidance of unnecessary macros.

## Features
---

#### No preprocessor abuse

Tests are defined using ordinary C++ without the use of the C preprocessor,
resulting in clearer code and greater flexibility.

#### Nested suites

Test suites can be nested arbitrarily deep. Group your tests however you like,
and let the suites set up and tear down your fixtures for you.

#### Declarative, matcher-based expectations

Expectations (assertions) are created using composable matchers, allowing you to
test complex things using a minimal set of core functions.

---

## A brief example

A picture is worth a thousand words, and code is probably worth a decent
fraction of that (let's say 100 words), so let's take a look:

```c++
#include <mettle.hpp>

using namespace mettle;

struct basic_data {
  int foo;
};

suite<basic_data> basic("basic suite", [](auto &_) {
  _.setup([](basic_data &) {
  });

  _.teardown([](basic_data &) {
  });

  _.test("a test", [](basic_data &) {
    expect(true, equal_to(true));
  });

  for(int i = 0; i < 4; i++) {
    _.test("test number " + std::to_string(i), [i](basic_data &) {
      expect(i % 2, equal_to(0));
    });
  }
});
```

## Dependencies

This project currently requires a C++14 compiler (for generic lambdas in the
matchers) and Boost (for argument parsing in the test runner). It's been tested
against clang 3.4 (get it from [http://llvm.org/apt/](http://llvm.org/apt/));
Ubuntu 13.10's version won't work!).
