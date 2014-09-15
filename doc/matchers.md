# Expectations and Matchers
---

## Expectations

Expectations are a way of checking that the state of your code matches your
expectation, similar to an `assert`. If the expectation is met, then everything
proceeds as normal. Otherwise, it throws an exception and aborts the current
test. Expectations are very simple to write:

```c++
expect(the_beast, equal_to(666));
```

If this expectation failed, you'd see something like the following logged to
your console (with the name of the failing test preceding it):

```
expected: 666
actual:   123
```

### Describing an expectation

You can also provide an optional description for an expectation to make it
easier to figure out what it was testing if you get a failure:

```c++
expect("is 'to mega therion?'", the_beast, equal_to(666));
```

If this expectation fails, you'll see a message like the previous failure, but
with the description shown as well:

```
is 'to mega therion?'
expected: 666
actual:   123
```

## Matchers

Above, you may have noticed the second argument to the expectation:
`equal_to(666)`. This is the expectation's *matcher*. Matchers are composable
functions that let you declaratively define what you expect a value to be. This
allows you to build complex expections from a relatively small set of simple
matchers, and frees you from having to manually describe every expectation. The
matchers do that for you!

## Built-in matchers

Mettle comes with a set of general-purpose matchers that should cover the most
common cases. We'll look at each of them below.

### Basic matchers

#### anything()

A matcher that always returns `true`. This is useful primarily as a placeholder,
e.g. when you can't know for sure what an expected value is going to be, but you
still want to test it.

#### is_not(*matcher*)

A matcher that negates another matcher.

#### describe(*matcher*, *desc*)

A matcher that overrides the description of another matcher. `desc` is a string
describing the resulting matcher. This can be useful for when the default
description is unclear or overly-verbose.

### Relational matchers

#### equal_to(*value*)

A matcher that returns `true` when the expected value is equal to `value`
according to the `==` operator.

`equal_to` is also the *implicit matcher*. Any time a matcher takes another
matcher as an argument, you can pass in a value, and it will be implicitly
converted to `equal_to(value)`. (Note that this doesn't work for the root
matcher; you need to explicitly say `equal_to(value)` in that case.

#### not_equal_to(*value*)

A matcher that returns `true` when the expected value is not equal to `value`
according to the `!=` operator.

#### greater(*value*)

A matcher that returns `true` when the expected value is greater than `value`
according to the `>` operator.

#### greater_equal(*value*)

A matcher that returns `true` when the expected value is greater than or equal
to `value` according to the `>=` operator.

#### less(*value*)

A matcher that returns `true` when the expected value is less than `value`
according to the `<` operator.

#### less_equal(*value*)

A matcher that returns `true` when the expected value is less than or equal to
`value` according to the `<=` operator.

### Arithmetic matchers

#### near_to(*value*[, *epsilon*])

A matcher that returns `true` when the expected value is approximately equal to
`value`, specifically when:

```c++
auto mag = std::max<T>(std::abs(expected), std::abs(actual));
std::abs(actual - expected) <= mag * epsilon;
```

If `epsilon` is not specified, it defaults to
`std::numeric_limits<T>::epsilon() * 10`. Note well: as with most functions that
check if two floating point numbers are approximately equal, this matcher will
likely fail if one of the values is zero. In that case, use `near_to_abs`.

#### near_to_abs(*value*, *tolerance*)

A matcher that returns `true` when the expected value is approximately equal to
`value`, specifically when `std::abs(actual - expected) <= tolerance`.

### Combinatoric matchers

#### any(*matchers...*)

A matcher that returns `true` when *any of* its composed matchers are true.

#### all(*matchers...*)

A matcher that returns `true` when *all of* its composed matchers are true.

#### none(*matchers...*)

A matcher that returns `true` when *none of* its composed matchers are true.

### Collection matchers

#### member(*matcher*)

A matcher that returns `true` when an item in a collection matches the composed
matcher.

#### each(*matcher*)

A matcher that returns `true` when *every* item in a collection matches the
composed matcher.

#### array(*matchers...*)

A matcher that returns `true` when the *ith* item in a collection matches the
*ith* composed matcher, *and* the number of items in the collection is equal to
the number of composed matchers.

#### sorted([*comparator*])

A matcher that returns `true` when the collection is sorted according to the
binary predicate `comparator` (or the less-than operator if `comparator` isn't
supplied).

### Exception matchers

Exception matchers work a bit differently from other matchers. Since we can't
catch an exception after the fact, we have to pass a function to our
expectation instead:

```c++
expect([]() { throw std::runtime_error("uh oh"); },
       thrown<std::runtime_error>("uh oh"));
```

#### thrown<*Type*>(*what_matcher*)

A matcher that returns `true` if an exception of type `Type` (or a subclass of
`Type`) is thrown and whose `what()` member function matches `what_matcher`.

#### thrown<*Type*>()

A matcher that returns `true` if an exception of type `Type` (or a subclass of
`Type`) is thrown; equivalent to: `thrown<Type>(anything())`.

#### thrown()

A matcher that returns `true` if an exception of *any* type `Type` is thrown.

#### thrown_raw<*Type*>(*matcher*)

A matcher that returns `true` if an exception of type `Type` (or a subclass of
`Type`) is thrown and whose value matches `matcher`.

## Writing your own matchers

mettle is designed to make it easy to write your own matchers to complement the
built-in suite of matchers. This makes it easier to test the state of objects
with complex properties. Once created, user-defined matchers can be composed as
normal with built-in matchers as you'd expect.

### Helper functions

#### make_matcher(*function*, *desc*)

The easiest way to create your own matcher is with the `make_matcher` function.
This takes two parameters: first, a function object that accepts a value of any
type, and returns a `bool` (with `true` naturally meaning a successful match);
and second, a string describing the matcher.

`make_matcher` returns a `basic_matcher<void, F>`, where `F` is the type of the
function, but it's easier to just deduce the return type. For instance, here's a
simple matcher that returns `true` when the actual value is 4:

```c++
auto match_four() {
  return make_matcher([](const auto &value) -> bool {
    return value == 4;
  }, "== 4");
}
```

#### make_matcher(*capture*, *function*, *prefix*)

You can also *capture* a value to use with your matcher; while you certainly
*can* capture the value via a lambda, passing the variable directly to
`make_matcher` allows it to be printed automatically when `desc()` is called. In
this overload, `function` works as above, except that it takes a second argument
for the captured object. The final argument, `prefix`, is a string that will be
prepended to the printed form of `capture`.

This overload of `make_matcher` returns a `basic_matcher<T, F>`, where `T` is
the type of the capture and `F` is the type of the function. Again, it's easier
to just deduce the return type. Here's an example of a matcher that returns
`true` when two numbers are off by one:

```c++
template<typename T>
auto off_by_one(T &&expected) {
  return make_matcher(
    expected,
    [](const auto &actual, const auto &expected) -> bool {
      auto x = std::minmax<T>(actual, expected);
      return x.second - x.first == 1;
    }, "off by 1 from "
  );
}
```

#### ensure_matcher(*thing*)

Sometimes, a matcher should be able to accept other matchers as an argument.
However, we also typically want to be able to pass in arbitrary objects as a
shorthand to implicitly call the `equal_to` matcher. In this case, we'd use
`ensure_matcher`.

`ensure_matcher` wraps an object with the `equal_to` matcher, or returns the
passed-in matcher if the object is already a matcher. As an example, let's
create a matcher that returns `true` if exactly one of its arguments is `true`:

```c++
template<typename T>
auto either(T &&a, T &&b) {
  auto a_matcher = ensure_matcher(std::forward<T>(a));
  auto b_matcher = ensure_matcher(std::forward<T>(b));

  return make_matcher([a_matcher, b_matcher](const auto &value) -> bool {
    return a_matcher(value) ^ b_matcher(value);
  }, a_matcher.desc() + " xor " + b_matcher.desc());
}
```

### Starting from scratch

For particularly complex matchers, `make_matcher` may not provide much value. In
these cases, you can instead build your own matcher from scratch. First, and
most importantly, all matchers must inherit from `matcher_tag`. This removes any
ambiguity between actual matchers and types that just have a similar interface.

As the previous section hints at, a matcher must also have a const overloaded
`operator ()` that takes a value of any type and returns a `bool`, and a const
`desc` function that returns a string description of the matcher.

A matcher made from scratch isn't much more complex than one made using the
helper functions above; most of the complexity will come from the behaviors you
define. Here's a simple example that *never* returns `true`:

```c++
struct nothing : matcher_tag {
  template<typename T>
  bool operator ()(const U &value) const {
    return false;
  }

  std::string desc() const {
    return "nothing";
  }
};
```

### Further examples

Naturally, many more examples of matchers can be found in mettle's own [source
code](https://github.com/jimporter/tree/master/include/mettle/matchers). Feel
free to consult these to get some ideas for how to implement your own matchers!
