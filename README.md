**Documentation for mettle is available at http://jimporter.github.io/mettle**

# mettle

**mettle** is a unit testing framework for C++14. It aims to maximize
readability and clarity, allowing you to focus on writing tests instead of
wrestling with the API.

## Features

#### Don't repeat yourself

Tests can be parameterized by value and by type, allowing you to reuse the same
tests with different preconditions.

#### Declarative, matcher-based expectations

Expectations (assertions) are created using composable matchers, allowing you to
test complex things using a minimal set of core functions.

#### Nested suites

Test suites can be nested arbitrarily deep. Group your tests however you like,
and let the suites set up and tear down your fixtures for you.

#### Aggregate all your tests

Write multiple, independent test files and aggregate them into a single list of
results with the `mettle` test runner.

## A Brief Example

A picture is worth a thousand words, and code's almost as good (I'm sure it's
worth at least 100 words), so let's take a look at a test file:

```c++
#include <mettle.hpp>
using namespace mettle;

suite<> basic("a basic suite", [](auto &_) {

  _.test("a test", []() {
    expect(true, equal_to(true));
  });

  for(int i = 0; i < 4; i++) {
    _.test("test number " + std::to_string(i), [i]() {
      expect(i % 2, less(2));
    });
  }

  subsuite<>(_, "a subsuite", [](auto &_) {
    _.test("a sub-test", []() {
      expect(true, equal_to(true));
    });
  });

});
```

## License

This library is [licensed](http://jimporter.github.io/mettle/license/) under the
BSD 3-Clause license.
