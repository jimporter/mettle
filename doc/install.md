# Installation

Broadly, mettle is composed of three parts: a set of header files, a shared
library (`libmettle.so` or `mettle.dll`), and a universal test driver
(`mettle`). Before we can start using mettle, we'll need to build and install
it.

## Dependencies

Before you get started with mettle, you'll need to install its dependencies:

* A C++14-compliant compiler (for generic lambdas and various standard library
  features)
     * [clang](http://clang.llvm.org/) + [libc++](http://libcxx.llvm.org/) 3.6+
     * [GCC](https://gcc.gnu.org/) + [libstdc++](https://gcc.gnu.org/libstdc++/)
       5.1+
     * [MSVC](http://www.visualstudio.com/) 2015+
* [Boost](http://www.boost.org/) 1.55+
* [bencode.hpp](https://github.com/jimporter/bencode.hpp)
* [bfg9000](http://jimporter.github.io/bfg9000/)

!!! note
    If you plan to use libc++,
    [this Gist](https://gist.github.com/jimporter/10442880) should help you
    build Boost to link to it.

### Header-only version

If you don't want to install all of the above dependencies and build the
binaries for mettle, you can use this library in a header-only mode by using
the following instead of the usual `#include <mettle.hpp>`:

```c++
#include <mettle/header_only.hpp>
```

However, the header-only version is quite limited and doesn't support any of the
command-line arguments described in [Running Tests](running-tests.md). In
addition, if you don't have Boost installed, you'll need a standard library that
includes `std::experimental::string_view`.

## Building and installing

Once you've installed all of mettle's dependencies, you can build mettle itself!
There are currently two ways to build mettle: via (experimental support for)
[bfg9000](https://github.com/jimporter/bfg9000), or via
[Make](https://www.gnu.org/software/make/).

### Building with bfg9000

!!! warning
    Building with `bfg9000` is currently experimental, but will become the sole
    method of building in 1.0.

Building with `bfg9000` is straightforward. Just run the following:

```sh
$ bfg9000 /path/to/mettle build
$ cd build
$ make install
```

You can specify the compiler to use and its options with the usual (Unix-style)
environment variables, such as `CXX` and `CXXFLAGS`. For further information
about how to use bfg9000, such as changing the build backend, see its
[documentation](https://github.com/jimporter/bfg9000).

### Building with Make

!!! warning
    Building with Make is deprecated and will be removed in 1.0. In addition,
    mettle's Makefile only works with compilers with `cc`-like arguments, so
    Visual Studio users should use the bfg9000 build file.

If you've already set up your environment variables to use your compiler of
choice, you should be able to run

```sh
$ make install
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
`make tests`. In that case, you can execute the tests as described later in
[Running Tests](running-tests.md).

## Building the examples

mettle comes with a series of examples to help show how to construct different
kinds of tests. Similar to the above, you can build all of these with
`make examples`.

## Building the documentation

mettle uses [MkDocs](http://www.mkdocs.org/) for its documentation. To build the
documentation, first install MkDocs, and then run `make doc-build`. You can also
run `make doc-serve` to run a test webserver with a preview of the
documentation.

## Cleaning the tree

As you might expect, you can also call `make clean` to clean the source tree of
generated files.
