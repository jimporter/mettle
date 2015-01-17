# Tutorial

So, you want to write some unit tests? This tutorial will walk you through the
steps needed to write, build, and run a simple test.

## Your first test

Let's get started by taking a look at the test file we used when [running
tests](running-tests.md). We'll discuss each part in detail below:

```c++
#include <mettle.hpp>
using namespace mettle;

suite<> first("my first suite", [](auto &_) {
  _.test("my first test", []() {
    expect(true, equal_to(true));
  });
});
```

### Dissecting the test file

First up, the obvious: we `#include <mettle.hpp>`, which imports all the code we
need to build and run simple tests: test suites, matchers, and a test runner.
With that out of the way, we can start defining our tests.

All tests are grouped into suites. Suites are created as global variables with
the type `mettle::suite<>`, and take a string name and a callback function
(typically a lambda). The callback lets us define our tests for the suite. It
takes a single argument, a reference to a
`mettle::suite_builder<mettle::expectation_failure>`, but since that's pretty
long, we conventionally just say `auto &` and use a generic lambda instead:

```c++
suite<> first("my first suite", [](auto &_) {
  /* ... */
});
```

!!! note
    There are lots of other options you can pass to the suite's constructor,
    like [fixtures](writing-tests.md#fixtures) or [test
    attributes](writing-tests.md#test-attributes), but we'll get to those
    later.

With the suite defined, now we just need to write our tests and add them via
the suite builder. Like suites, tests have both a string name and a callback
function, but this time the callback is the code to run when the test executes:

```c++
_.test(("my first test", []() {
  /* ... */
});
```

Inside our test function, we need to write some test code:

```c++
expect(true, equal_to(true));
```

This is an *expectation*. We'll discuss them in more detail
[later](matchers.md), but in short, they define the things we actually want to
*test* in our tests. This expectation makes sure that `true` is equal to `true`.
If it's not, the test will alert us to the fact so we can fix it (hopefully
before the universe finishes crashing down around us).

## Building the test

Building a test is straightforward. Since mettle provides its own test runner
with a `main()` function, the above source code is all you need for a
fully-operational test. Just compile the test like so:

```sh
$ clang++ -std=c++1y -lmettle -o test_first test_first.cpp
```

!!! note
    We need to compile in C++1y (aka C++14) mode, since mettle relies on some
    C++14 features to simplify writing tests.

## Running the test

Once you've built the test, you just need to run the binary and observe the
results. In this case, the results will look like:

```sh
$ ./test_first
.

1/1 test passed
```

The single dot (`.`) shows that our one and only test passed, which the summary
confirms. However, if the unthinkable has happened and our test fails, we'd see
something like this instead:

```sh
$ ./test_first
!

0/1 tests passed
  my first suite > my first test FAILED
    expected: true
    actual:   false
```

Here, we see an exclamation point (`!`) instead of a dot, to indicate a failed
test. At the end of the output, the details of the failure are shown. With this
information, we can hopefully diagnose the failure and fix the bug.

There are many more options that can be supplied to the test binary to alter its
output or how it runs tests. To learn more about those, see [Running
Tests](running-tests.md).
