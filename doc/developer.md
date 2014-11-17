# Developer Guide
---

If you're looking to learn about how mettle actually works, you've come to the
right place! This developer guide will describe mettle's internals and how
everything fits together.

## Basic structure

mettle's structure is a bit different from other test frameworks. First, like
some other test frameworks, individual (user-written) test files link to a
shared library (`libmettle.so`) to pull in common driver code.

However, in addition, the `mettle` test driver can be used to aggregate the
results of multiple test files. This lets you put your tests into multiple,
separate binaries to reduce compilation time and allow different kinds of tests
to be run all at once (e.g. normal unit tests and compilation-failure tests).

### Subprocesses

When running tests, mettle makes extensive use of subprocesses to isolate tests
as much as possible. First, the `mettle` driver forks/execs each individual test
binary, and these in turn (by default) fork for each test in the file. This
ensures that no one test can crash the entire framework. The *test process* is
also set as the process group leader for a new process group in order to ensure
that any subprocesses of it are killed, if necessary.

Additionally, if tests are set to time out after a certain period, two more
subprocesses are forked for each test: a *monitor process* and a *timer
process*. The monitor process forks both the timer process and the actual test
process and waits for the first one to finish. The timer process automatically
exits after the timeout expires, and if it exits before the test process, the
monitor process kills the test and sends a message that it timed out.

You might be thinking, "why not just use `alarm(2)` or `setitimer(2)` instead of
forking two extra times?" However, this would interact poorly with tests that
rely on functions like `sleep(3)`. Hence, in the interest of maximum isolation
of the test code, the timer is implemented as a subprocess.

## Suites

When creating a test suite, the most important argument to pass is the *creation
function* (conventionally a generic lambda). This function takes a
`suite_builder` (or a `subsuite_builder` for subsuites). These are both template
classes that let the user add tests or subsuites to the given suite.

Once the creation function returns, the suite is compiled into a
`runnable_suite` (subsuites, however, are merely compiled into an intermediary
form and stored in its parent's `(sub)suite_builder` instance). When the parent
is compiled, the subsuite and all of its tests are compiled as well, until it
gets to the root suite, at which point all tests and subsuites are
fully-compiled.

If using the global `suite` or `basic_suite` types, once the suite is
fully-compiled, it gets added to a global list of suites, held in
`detail::all_suites()` (subsuites, however, are contained within their parents).
This list is then used by the test driver to run all the tests.
