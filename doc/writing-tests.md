# Writing Tests
---

## Your First Test

So, you want to write some unit tests? Let's get started by taking a look at a
minimal test file. We'll discuss each part in detail below:

```c++
#include <mettle.hpp>

using namespace mettle;

suite<> first("my first suite", [](auto &_) {
  _.test("my first test", []() {
    expect(true, equal_to(true));
  });
});
```

### Building the Test

Before we start looking at how our tests are defined, let's build and run the
test to make sure everything's set up correctly.

Building the test is straightforward. Since mettle provides its own test runner
with a `main()` function, the above source code is all you need for a
fully-operational test. Just compile the test like so. Note that we compile in
C++1y (aka C++14) mode, since mettle relies on some C++14 features to make test
writing simpler:

```sh
clang++ -std=c++1y -Imettle/include -lboost_program_options -o test_first test_first.cpp
```

Once it's built, just run the binary and check your test results. If you like
pretty colors (I do!), you can pass `--color --verbose` to the executable.

### Dissecting the Test File

First up, the obvious: we `#include <mettle.hpp>`, which imports all the code we
need to build and run simple tests: test suites, matchers, and a test runner.
With that out of the way, we can start defining our tests.

Suites are created as global variables with the type `mettle::suite<>`, and take
a string name and a callback. You can optionally add some template type
parameters to specify [test fixtures](#fixtures). The callback lets us define
our tests for the suite. It takes a reference to a
`mettle::suite_builder<mettle::expectation_failure>` but we can just say
`auto &` and use a generic lambda instead.

With the suite defined, now we just need to write our tests and add them to the
suite via the test builder. Like suites, tests have both a string name and a
callback function, but this time the callback is the code to run when the test
executes.

Inside our test function, we need to write some test code:

```c++
expect(true, equal_to(true));
```

This is an *expectation*. We'll discuss them in more detail
[later](matchers.md), but in short, they define the things we actually want to
*test* in our tests. This expectation makes sure that `true` is equal to `true`.
If it's not, the test will alert us to the fact so we can fix it (hopefully
before the universe finishes crashing down around us).

### Skipped Tests

Some days, you just can't get a test to pass. While I can only assume this is
your fault, and that you should therefore feel bad until you fix it, you may
choose to skip the test for the time being:

```c++
_.skip_test("my broken test", []() {
  /* ... */
});
```

This will prevent the test from running and keep your test suite passing (with a
note that there are some skipped tests). But please, for everyone's sake, fix
your test! Thanks in advance.

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
