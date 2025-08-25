# Changes

## v0.2
in progress
{: .subtitle}

### New features
- Interleave skipped and failed tests in their original order in the summary log
- Capturing large arrays of trivial types for matchers is now much faster
- `expect` now takes a universal reference for the value to test to support
  matchers which need access to non-const objects (e.g. to call a non-const
  member function)
- New matchers `in_interval` and `exception_what`
- Add support for `char8_t`
- xUnit test output now includes file and line number in `<testcase>` tags

### Bug fixes
- Test failures across multiple runs are now correctly grouped in the summary
  results
- `bencode.hpp` is now installed alongside mettle when `--vendorize` is used
- Arithmetic matchers now use ADL to find `max` and `abs`, allowing custom
  arithmetic types to use their own implementations of these functions
- Print strings with unusual character types as an array of characters

### Breaking changes
- Implementation updated to require C++20
- `make_matcher` helper has been removed; use `basic_matcher` directly instead
- `METTLE_EXPECT` macro has been removed; use `expect` instead

---

## v0.1
2019-11-29
{: .subtitle}

- Initial release
- Support for defining assertions via composable matchers
- Tests can be groups into suites/subsuites, which can additionally be
  parameterized by type or value
- Allow aggregating the output of multiple test executables via the `mettle`
  driver
