# Expectations and Matchers

## Expectations

Expectations are a way of checking that the state of your code matches your
expectation, similar to an `assert`. If the expectation is met, then everything
proceeds as normal. Otherwise, it throws an exception and aborts the current
test. Expectations are very simple to write:

```c++
expect(the_beast, equal_to(666));
```

## Matchers

Above, you may have noticed the second argument to the expectation:
`equal_to(666)`. This is the expectation's *matcher*. Matchers are composable
functions that let you declaratively define what you expect a value to be. This
allows you to build complex expections from a relatively small set of simple
matchers, and frees you from having to manually describe every expectation. The
matchers do that for you!

### Built-in Matchers

Mettle comes with a set of general-purpose matchers that should cover the most
common cases. We'll look at each of them below.

#### Basic Matchers

##### `anything()`

A matcher that always returns `true`. This is useful primarily as a placeholder,
e.g. when you can't know for sure what an expected value is going to be, but you
still want to test it.

##### `is_not(matcher)`

A matcher that negates another matcher.

#### Boolean Matchers

##### `equal_to(value)`

A matcher that returns `true` when the expected value is equal to `value`
according to the `==` operator.

`equal_to` is also the *implicit matcher*. Any time a matcher takes another
matcher as an argument, you can pass in a value, and it will be implicitly
converted to `equal_to(value)`. (Note that this doesn't work for the root
matcher; you need to explicitly say `equal_to(value)` in that case.

##### `not_equal_to(value)`

A matcher that returns `true` when the expected value is not equal to `value`
according to the `!=` operator.

##### `greater(value)`

A matcher that returns `true` when the expected value is greater than `value`
according to the `>` operator.

##### `greater_equal(value)`

A matcher that returns `true` when the expected value is greater than or equal
to `value` according to the `>=` operator.

##### `less(value)`

A matcher that returns `true` when the expected value is less than `value`
according to the `<` operator.

##### `less_equal(value)`

A matcher that returns `true` when the expected value is less than or equal to
`value` according to the `<=` operator.

#### Combinatoric Matchers

##### `any_of(matchers...)`

A matcher that returns `true` when *any of* its composed matchers are true.

##### `all_of(matchers...)`

A matcher that returns `true` when *all of* its composed matchers are true.

#### Collection Matchers

##### `member(matcher)`

A matcher that returns `true` when an item in a collection matches the composed
matcher.

##### `each(matcher)`

A matcher that returns `true` when *every* item in a collection matches the
composed matcher.

##### `array(matchers...)`

A matcher that returns `true` when the *i*th item in a collection matches the
*i*th composed matcher, *and* the number of items in the collection is equal to
the number of composed matchers.

#### Exception Matchers

Exception matchers work a bit differently from other matchers. Since we can't
catch an exception after the fact, we have to pass a function to our
expectation instead:

```c++
expect([]() { throw std::runtime_error("uh oh"); },
       thrown<std::runtime_error>("uh oh"));
```

##### `thrown<Type>(what_matcher)`

A matcher that returns `true` if an exception of type `Type` (or a subclass of
`Type` is thrown and whose `what()` member function matches `what_matcher`.

##### `thrown<Type>()`

A matcher that returns `true` if an exception of type `Type` (or a subclass of
`Type` is thrown; equivalent to: `thrown<Type>(anything())`.

##### `thrown()`

A matcher that returns `true` if an exception of *any* type `Type` is thrown.

##### `thrown_raw<Type>(matcher)`

A matcher that returns `true` if an exception of type `Type` (or a subclass of
`Type` is thrown and whose value matches `what_matcher`.
