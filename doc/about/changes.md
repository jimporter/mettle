# Changes

## v0.2
in progress
{: .subtitle}

### New features
- Interleave skipped and failed tests in their original order in the summary log
- Capturing large arrays of trivial types for matchers is now much faster

### Bug fixes
- Test failures across multiple runs are now correctly groups in the summary
  results

### Breaking changes
- Implementation updated to require C++17
- `make_matcher` helper has been removed; use `basic_matcher` directly instead

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
