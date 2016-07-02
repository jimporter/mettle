# Installation

Broadly, mettle is composed of three parts: a set of header files, a shared
library (`libmettle.so` or `mettle.dll`), and a universal test driver
(`mettle`). Before we can start using mettle, we'll need to build and install
it.

## Just show me what to type!

Here are the necessary commands to build and install mettle, assuming you
already have Boost and a C++14 compiler (we'll discuss all of these parts in
more detail below):

```sh
$ cd path/to/mettle
$ scripts/vendorize_bencode.py
$ pip install bfg9000
$ 9k build/
$ cd build/
$ ninja install
```

## Dependencies

Before you get started with mettle, you'll need to install its dependencies:

* A C++14-compliant compiler (for generic lambdas and various standard library
  features)
    * [clang](http://clang.llvm.org/) 3.6+
    * [GCC](https://gcc.gnu.org/) 5.1+
    * [MSVC](https://www.visualstudio.com/) 2015+
* [Boost](http://www.boost.org/) 1.55+
* [bencode.hpp](https://github.com/jimporter/bencode.hpp)
* [bfg9000](https://jimporter.github.io/bfg9000/)

To simplify the installation of bencode.hpp, you can run
`scripts/vendorize_bencode.py` from your mettle source directory. This will
download and copy bencode.hpp to mettle's include directory, so you won't have
to install it yourself.

!!! note
    If you plan to use libc++, [this
    Gist](https://gist.github.com/jimporter/10442880) should help you
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
To build mettle, you'll first need to install
[bfg9000](https://jimporter.github.io/bfg9000/). The snippets below assume
you're using the [Ninja](https://ninja-build.org/) backend for bfg9000, but you
can replace this with any other build system bfg supports.

Building with bfg9000 is straightforward. Just run the following:

```sh
$ cd path/to/mettle/
$ 9k build/
$ cd build/
$ ninja install
```

You can specify the compiler to use and its options with the usual (Unix-style)
environment variables, such as `CXX` and `CXXFLAGS`. For further information
about how to use bfg9000, such as changing the build backend, see its
[documentation](https://jimporter.github.io/bfg9000/user/building/).

## Testing mettle

Mettle's own tests are written entirely in mettle. (It wouldn't be a very good
test framework otherwise!) To build and run all the tests, just call the
following from the build directory:

```sh
$ ninja test
```

If you'd rather build the tests *without* running them, you can call
`ninja tests`. In that case, you can execute the tests as described later in
[Running Tests](running-tests.md).

## Building the examples

mettle comes with a series of examples to help show how to construct different
kinds of tests. Similar to the above, you can build all of these with
`ninja examples`.

## Building the documentation

mettle uses [MkDocs](http://www.mkdocs.org/) for its documentation. To build the
documentation, first install MkDocs, and then run `ninja doc-build`. You can
also run `ninja doc-serve` to run a test webserver with a preview of the
documentation.
