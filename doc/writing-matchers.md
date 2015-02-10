# Writing Your Own Matchers

mettle is designed to make it easy to write your own matchers to complement the
built-in suite of matchers. This makes it easier to test the state of objects
with complex properties. Once created, user-defined matchers can be composed as
normal with built-in matchers as you'd expect.

## Helper functions

### make_matcher(*function*, *desc*)

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

### make_matcher(*capture*, *function*, *prefix*)

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
    std::forward<T>(expected),
    [](const auto &actual, const auto &expected) -> bool {
      auto x = std::minmax<T>(actual, expected);
      return x.second - x.first == 1;
    }, "off by 1 from "
  );
}
```

### ensure_matcher(*thing*)

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

## Starting from scratch

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

## Further examples

Naturally, many more examples of matchers can be found in mettle's own [source
code](https://github.com/jimporter/tree/master/include/mettle/matchers). Feel
free to consult these to get some ideas for how to implement your own matchers!
