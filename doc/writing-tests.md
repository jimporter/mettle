# Writing Tests

Below, we'll look at how test files are structured, how to create the tests
themselves, and how to group them together to perform higher-level operations on
them. Throughout this page, we'll assume that the following appears at the
beginning of our examples:

```c++
#include <mettle.hpp>
```

This includes *all* the headers necessary to use mettle. Alternately, you can
`#include <mettle/header_only.hpp>` if you prefer to use mettle without
compiling `libmettle.so` and the `mettle` universal driver.

If you'd rather include a smaller subset of headers, e.g. to improve compilation
speeds, you'll need the following to define tests and suites:

```c++
#include <mettle/suite.hpp> // The core of suite definition
#include <mettle/glue.hpp>  // Hooks up the suites with the expectations

// Choose only one of these:
#include <mettle/driver/lib_driver.hpp>    // The driver to go with libmettle.so
#include <mettle/driver/header_driver.hpp> // The header-only driver
```

## Defining suites

In mettle, all tests are grouped by suites. Suites are generally defined as
global variables (we'll see more ways to define suites later), and take two
arguments in the constructor: a string name, and a callback function taking a
reference to a `mettle::suite_builder<mettle::expectation_failure>`. Generally,
we just define the callback as a generic lambda:

```c++
mettle::suite<> my_suite("my suite", [](auto &_) {
  /* ... */
});
```

If you don't plan to use mettle's [expectations](matchers.md) for checking
program state, you can replace `mettle::suite<>` with
`mettle::basic_suite<my_exception>` to create a test suite using your own
exception type as the "canonical" exception.

### Tests

While we have a suite now, we still need to define some tests. Tests are,
without a doubt, the most important part of a *test* suite. We define our tests
inside our suite's callback function, using the `suite_builder` passed into it.
Like our suite, each test takes two arguments: a string name, and a callback
function defining the test. However, the test's callback takes no arguments:

```c++
mettle::suite<> my_suite("my suite", [](auto &_) {
  _.test("my test", []() {
    /* ... */
  });
});
```

Inside the test's callback function, we can write our actual test. If our code
throws an exception, the test will fail; otherwise, it passes. Most of your
tests should use *[expectations](matchers.md)* (similar to assertions) to
perform the actual tests. Expectations provide informative error messages if any
part of your test fails.

## Setup and teardown

Sometimes, you'll have a bunch of tests that all have the same setup and
teardown code. Test fixtures let you do this (mostly) automatically. If a test
suite has a `setup` or `teardown` function set, they'll run before (or after)
each test in the suite:

```c++
mettle::suite<> with_setup("my suite", [](auto &_) {
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

## Subsuites

When testing something particularly complex, you might find it useful to group
test suites together. You can do this by creating a subsuite inside a parent
suite:

```c++
mettle::suite<> with_subsuites("suite with subsuites", [](auto &_) {

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
mettle::suite<> with_subsuites("suite with subsuites", [](auto &_) {
  mettle::subsuite<>(_, "subsuite", [](auto &_) {
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

## Fixtures

Setup and teardown functions are of limited use without fixtures. Fixtures allow
you to safely write multiple, independent tests that test the same object. Each
test gets its own instance of the fixture, preventing it from affecting any of
the other tests. A fixture's lifetime is as follows:

1. Constructor the fixture using the default constructor.
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

mettle::suite<my_fixture> with_fixture("suite with a fixture", [](auto &_) {
  _.setup([](my_fixture &f) {
    f.i = 1;
  });

  _.test("test my fixture", [](my_fixture &f) {
    mettle::expect(f.i, equal_to(1));
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

Sometimes, the default constructor for a fixture isn't appropriate. In these
cases, you can use [fixture factories](#fixture-factories), discussed below.

### Nested fixtures

Like the nested setup and teardown functions, test fixtures are also
inherited in subsuites. This allows a parent suite to handle common fixtures for
a bunch of subsuites, reducing code duplication:

```c++
mettle::suite<int> nested_fixtures("suite with subsuites", [](auto &_) {
  _.setup([](int &i) {
    i = 1;
  });

  _.test("my parent test", [](int &i) {
    mettle::expect(i, equal_to(1));
  });

  mettle::subsuite<std::string>(_, "subsuite", [](auto &_) {
    _.setup([](int &i, std::string &s) {
      i++;
      s = "foo";
    });

    _.test("my subtest", [](int &i, std::string &s) {
      mettle::expect(i, equal_to(2));
      mettle::expect(s, equal_to("foo"));
    });
  });
});
```

As you can see above, subsuites inherit their parents' fixtures, much like they
inherit their parents' setup and teardown functions.

### Fixture factories

Sometimes, a fixture can't be constructed as is, e.g if the fixture isn't
default-constructible. In these cases, you can use a *fixture factory* to create
your fixture object with any parameters you like. A fixture factory is simply an
object with a templated `make<T>()` function:

```c++
struct my_factory {
  template<typename T>
  T make() {
    return T(12);
  }
};

mettle::suite<my_fixture> with_factory("suite", my_factory{}, [](auto &_) {
  /* ... */
});
```

In fact, "ordinary "fixtures use their own factory: `auto_factory`. The
following code snippets are equivalent:

```c++
mettle::suite<my_fixture>
without_auto_factory("suite", [](auto &_) {
  /* ... */
});

mettle::suite<my_fixture>
with_auto_factory("suite", mettle::auto_factory, [](auto &_) {
  /* ... */
});
```

Later, we'll learn more about fixture factories and how to
[transform](#transforming-fixture-types) their types.

## Parameterizing tests

While suites are a good way to group your tests together, sometimes you want to
run the *same* tests on several different types of objects. In this case, all
you need to do is specify *multiple* fixtures when defining a test suite. The
example below creates two test suites, one with a fixture of `int` and one with
a fixture of `float`:

```c++
mettle::suite<int, float> param_test("parameterized suite", [](auto &_) {
  _.test("my test", [](auto &fixture) {
    /* ... */
  });
});
```

This works just the same for subsuites as well:

```c++
mettle::suite<> param_sub_test("parameterized subsuites", [](auto &_) {
  mettle::subsuite<int, float>(_, "parameterized suite 1", [](auto &_) {
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

### Transforming fixture types

Earlier, we learned about [fixture factories](#fixture-factories). We can do
even more with them, though: a fixture factory's `make<T>()` can actually return
*any* type (including `void`!), not just `T`. This can be useful for more
complex tests, like testing a container type with several different element
types:

```c++
struct vector_factory {
  template<typename T>
  std::vector<T> make() {
    return {};
  }
};

mettle::suite<int, float> vector_suite("suite", vector_factory{}, [](auto &_) {
  _.test("empty()", [](auto &vec) {
    mettle::expect(vec.empty(), equal_to(true));
  });
});
```

### Type-only fixtures

As mentioned above, a fixture factory's `make<T>()` can return `void`. In this
case, the suite has no fixture whatsoever. This is primarily useful when you
want to parameterize on a list of types, but you don't want to automatically
instantiate the fixture object. The built-in fixture factory `type_only` handles
this for you. In particular, note the parameter-less test function:

```c++
mettle::suite<int, float> type_only("suite", mettle::type_only, [](auto &_) {
  _.test("empty()", []() {
    /* ... */
  });
});
```

### Getting the parameterized type

In some cases, you may want to know the parameterized type, e.g. if you'd like
to create your own instances of the object. You can retrieve this via the
`fixture_type` trait (or the `fixture_type_t` alias) like so:

```c++
mettle::suite<int, float> param_type("suite", mettle::type_only, [](auto &_) {
  using Fixture = mettle::fixture_type_t<decltype(_)>;

  _.test("my test", []() {
    /* use Fixture here */
  });
});
```

## Test attributes

For large projects with many tests, it can be useful to run only a subset of
them, instead of the entire collection. While splitting up tests by file can
help, it doesn't allow for very precise control of what tests get run. Instead,
you can apply attributes to your tests (or whole suites!) and filter on them.

### The *skip* attribute

mettle provides one built-in attribute: `mettle::skip`. As the name implies,
this attribute causes a test to be skipped by default. This can be useful when a
test is broken, since the test runner will keep track of the skipped tests as a
reminder that you need to go back and fix the test. You can also provide a
comment for the skipped test that will be shown in the test logs explaining why
it was skipped.

For more information about how to use the `skip` attribute, see [Using
Attributes](#using-attributes) below.

### Defining attributes

In addition to the built-in `skip` attribute, you can define your own
attributes. There are three basic kinds of attributes, differentiated by the
number of values each can hold: `mettle::bool_attr`, which holds 0 or 1 values;
`mettle::string_attr`, which holds exactly 1 value; and `mettle::list_attr`,
which holds 1 or more distinct values.

`bool_attr`s are somewhat special and can be given a default action
when they're encountered (either `mettle::attr_action::run` or
`mettle::attr_action::skip`). As you might be able to guess, the predefined
`skip` attribute is just a `bool_attr` whose action is `attr_action::skip`.

To define an attribute, you just need to create a global instance of one of the
aforementioned attribute kinds:

```c++
mettle::bool_attr slow("slow");
mettle::bool_attr busted("busted", attr_action::skip);
mettle::list_attr tags("tags");
```

### Using attributes

It's easy to apply attributes to your tests: simply create instances of each
kind of attribute you want, and pass them to the test creation function
immediately after the test name:

```c++
_.test("my test", {mettle::skip, slow("takes too long"), tags("cat", "goat")},
       [](auto &fixture) { /* ... */ });
```

This creates a `mettle::attributes` object that gets stored alongside the test.
As you might notice, `bool_attrs` can be implicitly converted to an attribute
instance, but other types require you to call them to list their values.

#### Suite attributes

Like tests, whole suites can have attributes associated with them; these are
applied the same way as for tests, and the list of attributes will automatically
be inherited by any children (tests or subsuites). The children can set their
own attributes, and all the attributes in the hierarchy will be combined
together for each test.

For `bool_attr`s and `string_attr`s, children with the same attribute as their
parents will *override* the parent's attribute, but for `list_attr`s, the
values from the parent and child will be merged:

```c++
mettle::suite<>
attr_suite("my suite", {mettle::skip("broken"), tags("cat")}, [](auto &_) {
  _.test("my test", {slow, mettle::skip("fixme"), tags("dog")}, []() {
    /* This test has the following attributes: slow, skip("fixme"), and
       tags("cat", "dog"). */
  });
});
```
