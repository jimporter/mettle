# Writing Tests
---

## Your First Test

So, you want to write some unit tests? Let's get started by taking a look at a
minimal test file. We'll discuss each part in detail below:

```c++
#include "mettle.hpp"

using namespace mettle;

suite<> first("my first suite", [](auto &_) {
  _.test("my first test", []() {
    expect(true, equal_to(true));
  });
});
```

**TODO**: Explanation goes here

### Building the Test

Building the test is straightforward. Since mettle provides its own test runner
with a `main()` function, the above source code is all you need for a
fully-operational test. Just compile the test like so:

```sh
clang++ -std=c++1y -Imettle/include -lboost_program_options -o test_first test_first.cpp
```

Once it's built, just run the binary and check your test results. If you like
pretty colors (I do!), you can pass `--color --verbose` to the executable.

## Fixtures

```c++
struct my_fixture {
  int i;
};

suite<my_fixture> basic("my first suite", [](auto &_) {
  _.test("my first test", [](my_fixture &fixture) {
    expect(true, equal_to(true));
  });
});
```

**TODO**

## Subsuites

**TODO**
