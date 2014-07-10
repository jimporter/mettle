# Running Tests
---

Before we start looking at how to write tests, let's build and run an existing
one to make sure everything's set up correctly.

## A sample test

First up, here's a minimalist test suite so that we have something to run. We'll
cover the syntax of this test later when we learn about [writing
tests](writing-tests.md).

```c++
#include <mettle.hpp>
using namespace mettle;

suite<> first("my first suite", [](auto &_) {
  _.test("my first test", []() {
    expect(true, equal_to(true));
  });
});
```

## Building the test

Building a test is straightforward. Since mettle provides its own test runner
with a `main()` function, the above source code is all you need for a
fully-operational test. Just compile the test like so. Note that we compile in
C++1y (aka C++14) mode, since mettle relies on some C++14 features to make test
writing simpler:

```sh
clang++ -std=c++1y -Imettle/include -lboost_program_options -o test_first test_first.cpp
```

Once it's built, just run the binary and check the test results.

## Command-line options

#### --verbose *N=1* (-v)

Show output of tests as they're being run. If `--verbose` isn't passed, the
verbosity is set to 0; if no value for *N* is specified, then the verbosity is
set to 1.

##### Verbosity 0

Don't show any output during the test run; only a summary after the fact will
be shown.

##### Verbosity 1

A single character for each test will be shown. `.` means a passed test, `!` a
failed test, and `_` a skipped test.

##### Verbosity 2

Show the full name of tests and suites as they're being run.

#### --color (-c)

Print test results in color. This is good if your terminal supports colors,
since it makes the resluts much easier to read!

#### --runs *N*

Run the tests a total of *N* times. This is useful for catching intermittent
failures. At the end, the summary will show the output of each failure for every
test.

#### --no-fork

By default, mettle forks its process to run each test, in order to detect
crashes during the execution of a test. To disable this, you can pass
`--no-fork`, and all the tests will run in the same process.

#### --show-terminal

Show the terminal output (stdout and stderr) of each test after it finishes. To
enable this, `--verbosity` must be at least 2, and `--no-fork` can't be
specified (if `--no-fork` is specified, the terminal output will just appear
in-line with the tests).
