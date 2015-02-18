# Built-in Matchers

Mettle comes with a set of general-purpose matchers that should cover the most
common cases. We'll look at each of them below.

## Core

### anything()

A matcher that always returns `true`. This is useful primarily as a placeholder,
e.g. when you can't know for sure what an expected value is going to be, but you
still want to test it.

### is_not(*matcher*)

A matcher that negates another matcher.

### describe(*matcher*, *desc*)

A matcher that overrides the description of another matcher. `desc` is a string
describing the resulting matcher. This can be useful for when the default
description is unclear or overly-verbose.

### filter(*func*, *matcher*[, *desc*])

A matcher that filters the expected value through a function before passing it
to another matcher. This helps when creating complex matchers that test multiple
parts of a type, such as:

```c++
all( filter([](auto &&x) { return x.first;  }, equal_to("first")),
     filter([](auto &&x) { return x.second; }, greater(0)) );
```

Additionally, you may pass in a string for `desc`, which will be added as a
prefix to `matcher`'s description. This helps provide useful output explaining
what exactly `func` is doing to the expected value.

## Relational

### equal_to(*value*)

A matcher that returns `true` when the expected value is equal to `value`
according to the `==` operator.

!!! note
    `equal_to` is also the *implicit matcher*. Any time a matcher takes another
    matcher as an argument, you can pass in a value, and it will be implicitly
    converted to `equal_to(value)`. This doesn't work for the root matcher; you
    need to explicitly say `equal_to(value)` in that case.

### not_equal_to(*value*)

A matcher that returns `true` when the expected value is not equal to `value`
according to the `!=` operator.

### greater(*value*)

A matcher that returns `true` when the expected value is greater than `value`
according to the `>` operator.

### greater_equal(*value*)

A matcher that returns `true` when the expected value is greater than or equal
to `value` according to the `>=` operator.

### less(*value*)

A matcher that returns `true` when the expected value is less than `value`
according to the `<` operator.

### less_equal(*value*)

A matcher that returns `true` when the expected value is less than or equal to
`value` according to the `<=` operator.

## Arithmetic

### near_to(*value*[, *epsilon*])

A matcher that returns `true` when the expected value is approximately equal to
`value`, specifically when:

```c++
auto mag = std::max<T>(std::abs(expected), std::abs(actual));
std::abs(actual - expected) <= mag * epsilon;
```

If `epsilon` is not specified, it defaults to
`std::numeric_limits<T>::epsilon() * 10`.

!!! note
    As with most functions that check if two floating point numbers are
    approximately equal, this matcher will likely fail if one of the values is
    zero. In that case, use `near_to_abs`.

### near_to_abs(*value*, *tolerance*)

A matcher that returns `true` when the expected value is approximately equal to
`value`, specifically when `std::abs(actual - expected) <= tolerance`.

## Combinatoric

### any(*matchers...*)

A matcher that returns `true` when *any of* its composed matchers are true.

### all(*matchers...*)

A matcher that returns `true` when *all of* its composed matchers are true.

### none(*matchers...*)

A matcher that returns `true` when *none of* its composed matchers are true.

## Collection

### member(*matcher*)

A matcher that returns `true` when an item in a collection matches the composed
matcher.

### each(*matcher*)

A matcher that returns `true` when *every* item in a collection matches the
composed matcher.

### each(*range*, *meta_matcher*)

A matcher that returns `true` when each item in a collection matches the
corresponding item in *range* according to the matcher built from
`meta_matcher`. *range* can be any iterable collection, including a
`std::initializer_list`. This is roughly equivalent to:

```c++
array( meta_matcher(range[0]), meta_matcher(range[1]), ...
       meta_matcher(range[n]) )
```

!!! note
    When `meta_matcher` is a template function, be careful about how you pass it
    in. For instance, `equal_to<int>` will expect an rvalue-reference to an
    `int`. If `range` is an lvalue, you'll need to pass `equal_to<const int &>`
    instead.

### each(*begin*, *end*, *meta_matcher*)

A matcher that returns `true` when each item in a collection matches the
corresponding item in the range [`begin`, `end`) according to the matcher built
from `meta_matcher`.

### array(*matchers...*)

A matcher that returns `true` when the *ith* item in a collection matches the
*ith* composed matcher, *and* the number of items in the collection is equal to
the number of composed matchers.

### tuple(*matchers...*)

Similar to `array`, but applies to tuples instead (or anything else with a
`std::get<I>()` overload). This matcher returns `true` when the *ith* item in a
tuple matches the *ith* composed matcher. Since the size of a tuple can be
determined at compile time, a length mismatch between the tuple and the number
of composed matchers will cause a compiler error.

### sorted([*comparator*])

A matcher that returns `true` when the collection is sorted according to the
binary predicate `comparator` (or the less-than operator if `comparator` isn't
supplied).

## Exception

Exception matchers work a bit differently from other matchers. Since we can't
catch an exception after the fact, we have to pass a function to our
expectation instead:

```c++
expect([]() { throw std::runtime_error("uh oh"); },
       thrown<std::runtime_error>("uh oh"));
```

### thrown<*Type*>(*what_matcher*)

A matcher that returns `true` if an exception of type `Type` (or a subclass of
`Type`) is thrown and whose `what()` member function matches `what_matcher`.

### thrown<*Type*>()

A matcher that returns `true` if an exception of type `Type` (or a subclass of
`Type`) is thrown; equivalent to: `thrown<Type>(anything())`.

### thrown()

A matcher that returns `true` if an exception of *any* type `Type` is thrown.

### thrown_raw<*Type*>(*matcher*)

A matcher that returns `true` if an exception of type `Type` (or a subclass of
`Type`) is thrown and whose value matches `matcher`.

## Death

!!! note
    These matchers are currently unusable on Windows. Sorry about that!

The most kvlt of all matchers, death matchers check that a function would cause
the process to terminate, either by signalling or by exiting. (These matchers
will fork a child process before calling the function so that the test
framework doesn't terminate.) Like [exception matchers](#exception-matchers),
death matchers require a function to be passed to the expectation:

```c++
expect([]() { abort(); }, killed(SIGABRT));
```

### killed([*matcher*])

A matcher that returns `true` if the function terminated the process via a
signal. If `matcher` is specified, `killed` will only return `true` if the
signal that was raised matches `matcher`.

### exited([*matcher*])

A matcher that returns `true` if the function terminated the process by exiting
(i.e. with `exit`, `_exit`, or `_Exit`). If `matcher` is specified, `exited`
will only return `true` if the exit status matches `matcher`.
