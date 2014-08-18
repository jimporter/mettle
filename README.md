# mettle

**mettle** is a unit testing framework for C++14. It aims to maximize
readability and clarity, allowing you to focus on writing tests instead of
wrestling with the API.

## Documentation

Documentation for mettle is available at http://jimporter.github.io/mettle

## Features

#### Don't repeat yourself

Tests can be parameterized by value and by type, allowing you to reuse the same
tests with different preconditions.

#### Nested suites

Test suites can be nested arbitrarily deep. Group your tests however you like,
and let the suites set up and tear down your fixtures for you.

#### Declarative, matcher-based expectations

Expectations (assertions) are created using composable matchers, allowing you to
test complex things using a minimal set of core functions.

#### No preprocessor abuse

Tests are defined using ordinary C++ without the use of the C preprocessor,
resulting in clearer code and greater flexibility.

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

  _.skip_test("a skipped test", []() {
    expect(3, any(1, 2, 4));
  });

  for(int i = 0; i < 4; i++) {
    _.test("test number " + std::to_string(i), [i]() {
      expect(i % 2, less(2));
    });
  }

});
```

## Dependencies

This project currently requires a C++14 compiler (for generic lambdas in the
matchers), [Boost](http://www.boost.org/) (for program_options and iostreams),
and [bencode.hpp](https://github.com/jimporter/bencode.hpp). It's been tested
against clang 3.4 (get it from [http://llvm.org/apt/](http://llvm.org/apt/);
Ubuntu 13.10's version won't work!).

## License

This library is licensed under the BSD 3-Clause license.
