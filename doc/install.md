# Installation

Broadly, mettle is composed of three parts: a set of header files, a shared
library (`libmettle.so` or `mettle.dll`), and a universal test driver
(`mettle`). Before we can start using mettle, we'll need to build and install
it.

## Dependencies

Before you get started with mettle, you'll need to install its dependencies:

* A C++14-compliant compiler (for generic lambdas and various standard library
  features)
     * [clang](http://clang.llvm.org/) + [libc++](http://libcxx.llvm.org/) 3.5
     * [GCC](https://gcc.gnu.org/) + [libstdc++](https://gcc.gnu.org/libstdc++/)
       5.0
     * [MSVC](http://www.visualstudio.com/) 2015 Preview
* [Boost](http://www.boost.org/)
* [bencode.hpp](https://github.com/jimporter/bencode.hpp)

!!! note
    If you plan to use libc++,
    [this Gist](https://gist.github.com/jimporter/10442880) should help you
    build Boost to link to it.

### Header-only version

If you're on Windows, or you don't want to install all of the above dependencies
(namely bencode.hpp and the non-header-only parts of Boost) and build the
binaries for mettle, you can use this library in a header-only mode by using the
following instead of the usual `#include <mettle.hpp>`:

```c++
#include <mettle/header_only.hpp>
```

However, the header-only version is quite limited and doesn't support any of the
command-line arguments described in [Running Tests](running-tests.md).

## Building and installing

!!! note
    Currently, the build system requires `make` and only works with compilers
    with `cc`-like arguments, so Visual Studio users are on their own for
    building for now. This will be fixed before 1.0 is released. Until then,
    sorry!

Once you've installed all of mettle's dependencies, you can build mettle itself!
If you've already set up your environment variables to use clang (or whatever
C++14-compliant compiler you like), you should be able to run

```sh
$ make && make install
```

to build and install mettle. Otherwise, you can specify the appropriate
environment variables on the command line or in `config.mk`. For example, you
could use the following `config.mk` file to build against clang and a
libsupc++-backed libc++:

```Makefile
CXX := clang++
CXXFLAGS := -std=c++1y -stdlib=libc++
LDFLAGS := -lsupc++
```

You can also specify `PREFIX` to change where mettle is installed.

## Testing mettle

Mettle's own tests are written entirely in mettle. (It wouldn't be a very good
test framework otherwise!) To build and run all the tests, just call:

```sh
$ make test
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
documentation, first install MkDocs, and then run `make build-doc`. You can also
run `make serve-doc` to run a test webserver with a preview of the
documentation.

## Cleaning the tree

As you might expect, you can also call `make clean` to clean the source tree of
generated files.
