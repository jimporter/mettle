# Running Tests

Once you've built an executable test (if you haven't built one, be sure to read
the [tutorial](tutorial.md) first), you'll find that there are a bunch of
options for running it. There are two main ways to run a test executable.

## Running the test executable directly

For small projects, you may have only a single test executable. In this case,
you can simply run the file directly:

```sh
$ ./test_my_code
```

[Below](#command-line-options), we'll see a list of the options we can pass to
the text executable, letting you tailor the output or how the tests are run.

## Using the *mettle* universal test driver

For testing larger projects, it's generally recommended to divide your test
suites into multiple `.cpp` files and compile them into separate binaries. This
allows you to improve build times and to better isolate tests from each other.

When doing so, you can use the `mettle` executable to run all of your individual
binaries at once. The interface is much like that of the individual binaries,
and all of the command-line options above work with the `mettle` executable as
well. To specify which of the test binaries to run, just pass their filenames to
`mettle`:

```sh
$ mettle test_file1 test_file2
```

Like the individual test executables, the `mettle` driver accepts several
[options](#command-line-options) to modify its behavior and output.

### Forwarding arguments

You can also pass along arguments to a test binary by quoting the binary:

```sh
$ mettle test_file1 "caliber test_foo.cpp test_bar.cpp"
```

This executes `test_file1` and then executes `caliber` with two arguments:
`test_foo.cpp` and `test_bar.cpp`. You can also pass wildcards (globs) inside
quoted arguments:

```sh
$ mettle test_file1 "caliber test_*.cpp"
```

## Command-line options

### Generic options

#### `--help` (`-h`) { #help-option }

Show help and usage information.

#### `--version { #version-option }

Show the current version of mettle.

### Driver options

#### <code>--timeout *N*</code> (`-t`) { #timeout-option }

Time out and fail any tests that take longer than *N* milliseconds to execute.

!!! warning
    [`--no-subproc`](#no-subproc-option) can't be specified while using this
    option.

#### <code>--test *REGEX*</code> (`-T`) { #test-option }

Filter the tests that will be run to those matching a regex. If `--test` is
specified multiple times, tests that match *any* of the regexes will be run.

#### <code>--attr *ATTR*[=*VALUE*],...</code> (`-a`) { #attr-option }

Filter the tests that will be run based on the tests'
[attributes](writing-tests.md#test-attributes). Tests matching each of the
attributes (and, optionally, values) will be run by the test driver. If `--attr`
is specified multiple times, tests that match *any* of the filters will be run.
You can also prepend *ATTR* with `!` to negate that test. Let's look at some
examples to get a better sense of how this works:

*   `--attr slow`

    Run only tests with the attribute `slow`.

*   `--attr protocol=http`

    Run only tests with the attribute `protocol` having a value of `http`.

*   `--attr '!slow'`

    Run only tests *without* the attribute `slow`. (Note the use of quotation
    marks, since many shells treat `!` as a special character.)

*   `--attr '!protocol=http'`

    Run only tests *without* the attribute `protocol`, or which have `protocol`
    set to something other than `http`.

*   `--attr slow,protocol=http`

    Run only tests that match both attributes.

*   `--attr slow --attr protocol=http`

    Run tests that match either attribute.

#### `--no-subproc` { #no-subproc-option }

By default, mettle creates a subprocess for each test, in order to detect
crashes during the execution of a test. To disable this, you can pass
`--no-subproc`, and all the tests will run in the same process. This option can
only be specified for the individual test binaries, *not* for the `mettle`
driver.

### Output options

#### <code>--output *FORMAT*</code> (`-o`) { #output-option }

Set the output format for the test results. If `--output` isn't passed, the
format is set to *brief*. The available formats are:

* `silent`: Don't show any output during the test run; only a summary after the
  fact will be shown.
* `counter`: Show a single line per run counting up the total number of passed,
  failed, and skipped tests.
* `brief`: A single character for each test will be shown. `.` means a passed
  test, `!` a failed test, and `_` a skipped test.
* `verbose`: Show the full name of tests and suites as they're being run.
* `xunit`: Log the test results in xUnit format to the file specified by
  [`--file`](#file-option).

#### <code>--color [*WHEN*]</code> (`-c`) { #color-option }

Print test results in color. This is good if your terminal supports colors,
since it makes the results much easier to read! `WHEN` can be one of `always`
(the default if you don't explicitly specify `WHEN`), `never`, or `auto`.

#### <code>--runs *N*</code> (`-n`) { #runs-option }

Run the tests a total of *N* times. This is useful for catching intermittent
failures. At the end, the summary will show the output of each failure for every
test.

#### `--show-terminal` { #show-terminal-option }

Show the terminal output (stdout and stderr) of each test after it finishes. To
enable this, [`--no-subproc`](#no-subproc-option) can't be specified (if
`--no-subproc` *is* specified, the terminal output will just appear in-line with
the tests).

#### `--show-time` { #show-time-option }

Show the duration (in milliseconds) of each test as it runs, as well as the
total time of the entire job.

#### <code>--file [*FILE*]</code> { #file-option }

The file to print test results to; only applies to the `xunit` [output
format](#output-option). Defaults to `mettle.xml`.
