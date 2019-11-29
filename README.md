# mettle

[![Documentation][documentation-image]][documentation-link]
[![Travis build status][travis-image]][travis-link]
[![Appveyor build status][appveyor-image]][appveyor-link]

**mettle** is a "batteries included" unit testing framework for C++14. Its
mission is to provide a full toolbox to address your testing needs and to look
good doing it.

## Features

#### Build your own assertions

Expectations (assertions) are defined using composable matchers that
automatically generate human-readable output, ensuring even complex objects are
easy to test.

#### Nest your tests

Suites group your tests together and can be nested as deeply as you need,
so you can use their hierarchy to set up and tear down your fixtures for you.

#### Don't repeat yourself

Type- and value-parameterized tests let you write your tests once and apply them
to multiple implementations or preconditions.

#### Aggregate everything

The `mettle` universal test runner makes it a snap to write multiple,
independent test files – even ones running completely different kinds of tests –
and aggregate them into a single list of results.

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

This library is licensed under the [BSD 3-Clause license](LICENSE).

[documentation-image]: https://img.shields.io/badge/docs-mettle-blue.svg
[documentation-link]: https://jimporter.github.io/mettle/
[travis-image]: https://travis-ci.org/jimporter/mettle.svg?branch=master
[travis-link]: https://travis-ci.org/jimporter/mettle
[appveyor-image]: https://ci.appveyor.com/api/projects/status/76hye1aj81qg1nqg/branch/master?svg=true
[appveyor-link]: https://ci.appveyor.com/project/jimporter/mettle/branch/master
