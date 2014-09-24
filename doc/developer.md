# Developer Guide
---

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
ensures that no one test can crash the entire framework.

Additionally, if tests are set to time out after a certain period, two more
subprocesses are forked for each test: a *monitor process* and a *timer
process*. The monitor process forks both the timer process and the actual test
process and waits for the first one to finish. The timer process automatically
exits after the timeout expires, and if it exits before the test process, the
monitor process kills the test and sends a message that it timed out.
