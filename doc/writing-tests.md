# Writing Tests
---

## Your first test

So, you want to write some unit tests? Let's get started by taking a look at the
test file we used when [running tests](running-tests.md). We'll discuss each
part in detail below:

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

### Skipped tests

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

## Setup and teardown

Sometimes, you'll have a bunch of tests that all have the same setup and
teardown code. Test fixtures let you do this (mostly) automatically. If a test
suite has a `setup` or `teardown` function set, they'll run before (or after)
each test in the suite:

```c++
suite<> with_setup("my suite", [](auto &_) {
  _.setup([]() {
    /* ... */
  });

  _.teardown([]() {
    /* ... */
  });

  _.test("my test", []() {
    /* ... */
  });
});
```

## Fixtures

Setup and teardown functions are of limited use without fixtures. Fixtures allow
you to safely share data between tests. A fixture's lifetime is as follows:

1. Construct the fixture.
2. Pass the fixture by reference to the setup function (if defined).
3. Pass the fixture by reference to the test function.
4. Pass the fixture by reference to the teardown function (if defined).
5. Destruct the fixture.

Declaring a fixture is simple. Just pass the type of your fixture in the
template parameters of your `mettle::suite` object:

```c++
struct my_fixture {
  int i;
};

suite<my_fixture> with_fixture("suite with a fixture", [](auto &_) {
  _.setup([](my_fixture &f) {
    f.i = 1;
  });

  _.test("test my fixture", [](my_fixture &f) {
    expect(f.i, equal_to(1));
  });
});
```

Astute readers will notice that a test fixture could easily be used to replace
the `setup` and `teardown` functions by using RAII. However, both options are
supported, since it's often simpler to write a setup/teardown code than to write
a less-flexible helper class. For instance, your fixture might be a database
object from your production code that you want to add some test records to for
testing. Rather than wrapping the database in a helper, you can just add the
test records in `setup`.

## Subsuites

When testing something particularly complex, you might find it useful to group
test suites together. You can do this by creating a subsuite inside a parent
suite:

```c++
suite<> with_subsuites("suite with subsuites", [](auto &_) {

  _.template subsuite<>("subsuite", [](auto &_) {
    _.test("my subtest", []() {
      /* ... */
    });
  });

});
```

You've probably noticed that we had to type `template subsuite<>` when declaring
our subsuite. This is because, as you may recall, our suite's callback uses a
generic lambda, and so `_` is a *dependent type*. Template member functions of a
dependent type must be disambiguated with the `template` keyword. We could
either redefine our lambda to no longer be generic, or just use the
`mettle::subsuite` helper:

```c++
suite<> with_subsuites("suite with subsuites", [](auto &_) {
  subsuite<>(_, "subsuite", [](auto &_) {
    /* ... */
  });
});
```

### Nested setup and teardown

As you might imagine, a test in a subsuite uses not only the subsuite's setup
and teardown functions, but inherits the parent suite's as well (and so on up
the tree). When executing a test in a subsuite, the test runner will walk down
the suite hierarchy, calling each setup function in turn before running the
test. After finishing the test, it will walk back up the tree calling each
teardown function.

For a two-level hierarchy, this is what would happen for each test in the
subsuite:

1. Call the parent suite's setup function (if defined).
2. Call the subsuite's setup function (if defined).
3. Run the test function
4. Call the subsuite's teardown function (if defined).
5. Call the parent suite's teardown function (if defined).

### Nested fixtures

Like the nested setup and teardown functions, test fixtures are also
inherited in subsuites. This allows a parent suite to handle common fixtures for
a bunch of subsuites, reducing code duplication:

```c++
suite<int> nested_fixtures("suite with subsuites", [](auto &_) {
  _.setup([](int &i) {
    i = 1;
  });

  _.test("my parent test", [](int &i) {
    expect(i, equal_to(1));
  });

  subsuite<std::string>(_, "subsuite", [](auto &_) {
    _.setup([](int &i, std::string &s) {
      i++;
      s = "foo";
    });

    _.test("my subtest", [](int &i, std::string &s) {
      expect(i, equal_to(2));
      expect(s, equal_to("foo"));
    });
  });
});
```

As you can see above, subsuites inherit their parents' fixtures, much like they
inherit their parents' setup and teardown functions.

## Parameterizing tests

While suites are a good way to group your tests together, sometimes you want to
run the *same* tests on several different types of objects. In this case, all
you need to do is specify *multiple* fixtures when defining a test suite. The
example below creates two test suites, one with a fixture of `int` and one with
a fixture of `float`:

```c++
suite<int, float> param_test("parameterized suite", [](auto &_) {
  _.test("my test", [](auto &fixture) {
    /* ... */
  });
});
```

This works just the same for subsuites as well:

```c++
suite<> param_sub_test("parameterized subsuites", [](auto &_) {
  subsuite<int, float>(_, "parameterized suite 1", [](auto &_) {
    /* ... */
  });

  _.template subsuite<int, float>(_, "parameterized suite 2", [](auto &_) {
    /* ... */
  });
});
```

One subtle difference you may have noticed is that now, our test definitions
use a generic lambda: `[](auto &fixture) { /* ... */ }`. As you might imagine,
this allows the test function to accept a fixture of either an `int` or a
`float` and to do the usual thing when a template is instantiated. Of course,
you don't *always* need to use `auto` here; if all of your fixtures inherit from
a common base type, you can use an ordinary lambda that takes a reference to the
base type.
