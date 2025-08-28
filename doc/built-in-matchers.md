# Built-in Matchers

Mettle comes with a set of general-purpose matchers that should cover the most
common cases. We'll look at each of them below. The entire set of matchers can
be included via `#include <mettle/matchers.hpp>`, or you can include just the
categories you need by including the appropriate file (e.g.
`#include <mettle/matchers/combinatoric.hpp`).

## Core
*&lt;mettle/matchers/core.hpp&gt;*
{: .subtitle}

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
*&lt;mettle/matchers/relational.hpp&gt;*
{: .subtitle}

### equal_to(*value*)

A matcher that returns `true` when the actual value is equal to `value`
according to the `==` operator.

!!! note
    `equal_to` is also the *implicit matcher*. Any time a matcher takes another
    matcher as an argument, you can pass in a value, and it will be implicitly
    converted to `equal_to(value)`. This doesn't work for the root matcher; you
    need to explicitly say `equal_to(value)` in that case.

### not_equal_to(*value*)

A matcher that returns `true` when the actual value is not equal to `value`
according to the `!=` operator.

### greater(*value*)

A matcher that returns `true` when the actual value is greater than `value`
according to the `>` operator.

### greater_equal(*value*)

A matcher that returns `true` when the actual value is greater than or equal to
`value` according to the `>=` operator.

### less(*value*)

A matcher that returns `true` when the actual value is less than `value`
according to the `<` operator.

### less_equal(*value*)

A matcher that returns `true` when the actual value is less than or equal to
`value` according to the `<=` operator.

### in_interval(*low*, *high*[, *bounds*])

A matcher that returns `true` when the actual value is between `low` and `high`.
`bounds` can be one of the following:

* `interval::closed`: expects `low <= actual <= high`
* `interval::left_open`: expects `low < actual <= high`
* `interval::right_open`: expects `low <= actual < high` (the default)
* `interval::open`: expects `low < actual < high`

## Arithmetic
*&lt;mettle/matchers/arithmetic.hpp&gt;*
{: .subtitle}

### near_to(*value*[, *epsilon*])

A matcher that returns `true` when the actual value is approximately equal to
`value`, specifically when:

```c++
auto mag = std::max<T>(std::abs(expected), std::abs(actual));
std::abs(actual - expected) <= mag * epsilon;
```

If `epsilon` is not specified, it defaults to
`std::numeric_limits<T>::epsilon() * 10`.

!!! warning
    As with most functions that check if two floating point numbers are
    approximately equal, this matcher will likely fail if one of the values is
    zero. In that case, use `near_to_abs`.

### near_to_abs(*value*, *tolerance*)

A matcher that returns `true` when the actual value is approximately equal to
`value`, specifically when `std::abs(actual - expected) <= tolerance`.

## Regular expression
*&lt;mettle/matchers/regex.hpp&gt;*
{: .subtitle}

### regex_match(*ex*[, *syntax*, *match*])

A matcher that returns `true` if the regex `ex` matches the entirety of the
actual value. If `syntax` or `match` are specified, the matcher uses those for
the regex's syntax and match flags, respectively.

### regex_search(*ex*[, *syntax*, *match*])

A matcher that returns `true` if the regex `ex` matches a subsequence of the
actual value. If `syntax` or `match` are specified, the matcher uses those for
the regex's syntax and match flags, respectively.

!!! note
    Since `std::regex` objects don't provide access to a string representation
    of the expression, the `ex` parameter in the above matchers should be a
    string, not a regex. This allows the matcher to print the regex to the
    console if the matcher fails.

## Combinatoric
*&lt;mettle/matchers/combinatoric.hpp&gt;*
{: .subtitle}

### any(*matchers...*)

A matcher that returns `true` when *any of* its composed matchers are true.

### all(*matchers...*)

A matcher that returns `true` when *all of* its composed matchers are true.

### none(*matchers...*)

A matcher that returns `true` when *none of* its composed matchers are true.

## Memory
*&lt;mettle/matchers/memory.hpp&gt;*
{: .subtitle}

### dereferenced(*matcher*)

A matcher that checks if the actual value can be dereferenced and then passes
the dereferenced value to the composed matcher.

## Collection
*&lt;mettle/matchers/collection.hpp&gt;*
{: .subtitle}

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

!!! warning
    When `meta_matcher` is a template function, be careful about how you pass it
    in. For instance, `equal_to<int>` will expect an rvalue-reference to an
    `int`. If `range` is an lvalue, you'll need to pass `equal_to<const int &>`
    instead.

### each(*begin*, *end*, *meta_matcher*)

A matcher that returns `true` when each item in a collection matches the
corresponding item in the range \[`begin`, `end`\) according to the matcher
built from `meta_matcher`.

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

### permutation(*range*[, *predicate*])

A matcher that returns `true` if a permutation of `range` exists that equals
the collection according for the binary predicate `predicate` (or the equality
operator if `predicate` isn't supplied).

### permutation(*begin*, *end*[, *predicate*])

A matcher that returns `true` if the range \[`begin`, `end`\) is a permutation
of the collection for the binary predicate `predicate` (or the equality operator
if `predicate` isn't supplied).

## Exception
*&lt;mettle/matchers/exception.hpp&gt;*
{: .subtitle}

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
This is equivalent to `thrown_raw<Type>(exception_what(what_matcher))`.

### thrown<*Type*>()

A matcher that returns `true` if an exception of type `Type` (or a subclass of
`Type`) is thrown; equivalent to: `thrown<Type>(anything())`.

### thrown()

A matcher that returns `true` if an exception of *any* type `Type` is thrown.

### exception_what(*what_matcher*)

A matcher that returns `true` if the value is an exception whose `what()` member
function matching `what_matcher`.

### thrown_raw<*Type*>(*matcher*)

A matcher that returns `true` if an exception of type `Type` (or a subclass of
`Type`) is thrown and whose value matches `matcher`.

## Death
*&lt;mettle/matchers/death.hpp&gt;*
{: .subtitle}

!!! warning
    Because these matchers deal with process creation/termination, they can
    cause issues with software that relies on exit handlers (e.g. code coverage
    tools). In addition, these matchers are currently unsupported on Windows.
    Sorry about that!

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
