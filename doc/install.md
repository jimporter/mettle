# Installation
---

Broadly, mettle is composed of three parts: a set of header files, a shared
library (`libmettle.so`), and a universal test driver (`mettle`). Before we can
start using mettle, we'll need to build and install it.

## Dependencies

First, and most importantly, mettle requires a C++14-compliant compiler (for
generic lambdas and generalized `constexpr`) and a POSIX environment (mainly
for `fork`; this dependency will be eliminated once Visual Studio has more
complete C++14 support). It's been tested against clang 3.4 (if you're using
Ubuntu 13.10, make sure get clang from
[http://llvm.org/apt/](http://llvm.org/apt/), since Ubuntu's version won't
work!). You may also need to use [libc++](http://libcxx.llvm.org/), since at the
time of this writing, libstdc++ lacked some necessary library features for
mettle.

In addition, you'll need [Boost](http://www.boost.org/) (for program_options and
iostreams) and [bencode.hpp](https://github.com/jimporter/bencode.hpp). If
you plan to use libc++, [this Gist](https://gist.github.com/jimporter/10442880)
should help you build Boost to link to it.

### Header-only version

If you'd like to get started quickly, and don't want to install the above
dependencies (namely Boost and bencode.hpp) or build the binaries for mettle,
you can use this library in a header-only mode by using the following instead of
the usual `#include <mettle.hpp>`:

```c++
#include <mettle/header_only.hpp>
```

However, the header-only version is quite limited and doesn't support any of the
command-line arguments described in [Running Tests](running-tests.md).

## Building and installing

Once you've installed all of mettle's dependencies, you can build mettle itself!
If you've already set up your environment variables to use clang (or whatever
C++14-compliant compiler you like), you should be able to run

```sh
make && make install
```

to build and install mettle. Otherwise, you can specify
the appropriate environment variables on the command line or in `config.mk`. For
example, you could use the following `config.mk` file to build against clang and
a libsupc++-backed libc++:

```Makefile
CXX := clang++
CXXFLAGS := -std=c++1y -stdlib=libc++
LDFLAGS := -lsupc++
```

You can also specify `PREFIX` to change where mettle is installed.

## Running tests

To build and run all the tests, just call:

```sh
make test
```

If you'd rather build the tests *without* running them, you can call
`make tests` or `make test/test_filename`, where `test_filename` is the name of
the test you'd like to build. In that case, you can execute the tests as
described later in [Running Tests](running-tests.md).

## Building the examples

mettle comes with a series of examples to help show how to construct different
kinds of tests. Similar to the above, you can build all of these with
`make examples`, or build a single file with `make examples/test_filename`.

## Building the documentation

mettle uses [MkDocs](http://www.mkdocs.org/) for its documentation. To build the
documentation, first install MkDocs, and then run `mkdocs build`. You can also
run `mkdocs serve` to run a test webserver with a preview of the documentation.

## Cleaning the tree

As you might expect, you can also call `make clean` to clean the source tree of
generated files.

