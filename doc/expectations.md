# Expectations

Expectations are a way of checking that the state of your code matches your
expectation, similar to an `assert`. If the expectation is met, then everything
proceeds as normal. Otherwise, it throws an exception and aborts the current
test.

## Declaring an expectation

Expectations are easy to write. Just call `mettle::expect` with the *actual*
value you'd like to test and a *matcher* object. We'll look at matchers in
detail [later](#matchers), but for now, let's just look at a simple matcher:
`mettle::equal_to(x)` creates a matcher that will return `true` when the actual
value passed to `expect` is equal to `x` (according to the definition of `==`
for the type(s) in question).  Here it is all put together:

```c++
mettle::expect(the_beast, mettle::equal_to(666));
```

If this expectation failed, you'd see something like the following logged to
your console (with the name of the failing [test case](writing-tests.md#tests)
preceding it):

```plain
/path/to/test_file.cpp:13
expected: 666
actual:   123
```

### Describing an expectation

You can also provide an description for an expectation to make it easier to
figure out what it was testing if you get a failure:

```c++
mettle::expect("is 'to mega therion'?", the_beast, mettle::equal_to(666));
```

If this expectation fails, you'll see a message like the previous failure, but
with the description shown as well:

```plain
is 'to mega therion'? (/path/to/test_file.cpp:13)
expected: 666
actual:   123
```

!!! note
    Currently, only GCC supports getting the filename and line number
    automatically for an expectation. However, as compilers add support for
    `std::experimental::source_location`, mettle will automatically switch to
    showing these values. However, in the meantime you can use the
    `METTLE_EXPECT` macro in place of `mettle::expect` if you need to see the
    filename and line number.


## Printing objects

In the examples above, the error output shows the expected and actual values.
This is easy for simple types, like integers, but what about complex
user-defined types? We could use the stream output operator `<<` (and indeed,
this option is supported), but there are times where you might want the debug
output to be different from the normal output. In addition, not every type has
(or should have) an overload of `<<`.

Instead, we use `mettle::to_printable` to ensure that our object will be
displayed appropriately if we need to print it. This function returns an object
(of unspecified type) that can be printed with the usual stream output operator.
You can also pass an iterator pair to `to_printable` to print the entire range.
Of course, feel free to overload `to_printable` for your own types to show
whatever information you need to help debug test failures.

!!! note
    Much like overloaded operators and standard library functions like
    `std::swap`, mettle uses argument-dependent lookup (ADL) to find the
    appropriate overload of `to_printable`. This is, of course, subject to all
    the usual concerns with ADL, so keep this in mind when using `to_printable`.

## What is a matcher?

[Above](#declaring-an-expectation), we talked briefly about matchers and
showed how to use the equality matcher: `mettle::equal_to(666)`. Before we
continue any further, we should define what exactly a matcher is. *Matchers* are
composable functions that let you declare what you expect a value to be. The
composability of matchers allows you to build complex expections from a
relatively small set of simple matchers. What's more, matchers automatically
produce human-readable descriptions of what exactly they're testing, freeing you
from having to manually describe every expectation!

In addition, mettle comes with a collection of [built-in
matchers](built-in-matchers.md) that can be used (or combined together) to
perform most common tests. However, if these don't suffice, you can always
[write your own](#writing-your-own-matchers).

### How matchers work

The internals of a matcher are actually pretty simple; they're just function
objects inheriting from `mettle::matcher_tag` and also providing a `desc()`
member function that returns a string describing the matcher. All matchers can
be called with a single argument (the *actual* value in the expectation) and
returns whether the match was successful or not. To illustrate this more
concretely, the following defines a very simple matcher that checks if an `int`
is 0:

```c++
struct int_zero : matcher_tag {
  bool operator ()(int actual) const {
    return actual == 0;
  }

  std::string desc() const {
    return "is 0";
  }
};
```

## Composing matchers

Many of the built-in matchers, such as the [combinatoric
matchers](built-in-matchers.md#combinatoric), are *higher-order matchers*. That
is, they are created by taking other matchers as arguments. This allows you to
build more complex matchers that test multiple aspects of an object at once. For
instance, you might want to check that a value is in the range \[2, 4\). With
the [`all`](built-in-matchers.md#all) matcher, this is easy:

```c++
using namespace mettle;
expect(x, all(greater_equal(2), less(4)));
```

Similarly, there are higher-order matchers that operate on
[collections](built-in-matchers.md#collection), allowing you, for example, to
test that all elements of a container meet a certain condition:

```c++
using namespace mettle;
expect(v, each(greater(0)));
```

By composing matchers together, you can express *all* the expected state of an
object in one place, making it clearer what exactly you're testing. Furthermore,
since all matchers provide a description of what they match, these higher-order
matchers can automatically tell you in human-readable form what they expect. For
instance, our test to check if a value is between 2 and 4 above might print
something like this if it fails:

```
expected: all(>=2, <4)
actual:   5
```

## Mismatch messages

As we saw above, for most simple expectations, a failure will cause the
matcher's description to be printed out along with the actual value supplied to
`expect`. However, for some matchers, just seeing the actual value isn't very
useful. For example, with the [`thrown`](built-in-matchers.md#exception)
matchers, you already know that the actual value is a function, so printing that
fact out doesn't help.

Instead, matchers can choose to return a mismatch message that provides a
morer useful, special-case description of the actual value (along with the usual
`bool` to indicate if the matcher succeeded). For the `thrown` matcher, we can
then return a description of the exception the function *actually* threw (if
any). In this case, the matcher returns a `mettle::match_result` object (a
`bool` and a `std::string` description) when called instead of a solitary
`bool`. This makes the output for a failed test much nicer in this case:

```
expected: threw std::runtime_error(what: "bad")
actual:   threw std::runtime_error(what: "worse")
```

A `match_result` is implicity convertible to and from a `bool`, so matchers that
return `match_result`s can easily be mixed with ones that return `bool`s. You
can also use the `!` operator to invert a `match_result` and preserve its
message.

!!! note
    Despite the name "mismatch message", it's useful to provide a message even
    on success, since a successful match can easily become a mismatch simply by
    using the `is_not` matcher.
