# Developer Guide

If you're looking to learn about how mettle actually works, you've come to the
right place! This developer guide will describe mettle's internals and how
everything fits together.

## Runner

mettle's structure is a bit different from other test frameworks. First, like
some other test frameworks, individual (user-written) test files link to a
shared library (`libmettle.so` / `mettle.dll`) to pull in common driver code.

However, in addition, the `mettle` test driver can be used to aggregate the
results of multiple test files. This lets you put your tests into multiple,
separate binaries to reduce compilation time and allow different kinds of tests
to be run all at once (e.g. normal unit tests and compilation-failure tests).

### Subprocesses

When running tests, mettle makes extensive use of subprocesses to isolate tests
as much as possible. First, the `mettle` driver creates a subprocess for each
individual test binary, and these in turn (by default) create a subprocess for
each test in the file. This ensures that no one test can crash the entire
framework. However, the particulars differ between POSIX systems and Windows.

#### POSIX

When the individual test binary forks for a specific test, the *test process* is
also set as the process group leader for a new process group. This ensures
that any subprocesses it spawns can be killed after the main process finishes.

Additionally, if tests are set to time out after a certain period, two more
subprocesses are forked for each test: a *monitor process* and a *timer
process*. The monitor process forks both the timer process and the actual test
process and waits for the first one to finish. The timer process automatically
exits after the timeout expires, and if it exits before the test process, the
monitor process kills the test and sends a message that it timed out.

You might be thinking, "why not just use `alarm(2)` or `setitimer(2)` instead of
forking two extra times?" However, this would interact poorly with tests that
rely on functions like `sleep(3)`. Hence, in the interest of maximum isolation
of the test code, the timer is implemented as a subprocess. Likewise, running
the timer in the parent process requires greater care when handling signals. In
any case, the current method has the significant benefit of being entirely
transparent to the parent process.

#### Windows

Since Windows is unable to fork a process, when the individual test binary
creates a subprocess for a test, it simply reruns itself with a different set of
command-line arguments, indicating that only a specific test should be run. This
subprocess is immediately placed into a new job, ensuring that - like the POSIX
version's process groups - any subprocesses it spawns can be killed after the
main process finishes.

Unlike the POSIX version, if tests are set to time out, we simply create a timer
event and wait for it while we're waiting for the test process to end. If the
timer's event fires first, we know to kill the test process. Since the timer is
run in the parent process, this provides the same level of isolation as the
POSIX version, but at the expense of some extra management; the parent process
must now pay specific attention to the timer event instead of assuming that its
child process will handle it.

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
