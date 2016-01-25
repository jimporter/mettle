#include <mettle.hpp>
using namespace mettle;

struct int_factory {
  template<typename T>
  T make() const {
    return {1, 2, 3};
  }
};

// XXX: Enable these tests on MSVC (they trigger an ICE somewhere).
#if !defined(_MSC_VER) || defined(__clang__)

suite<> test_string_alg("string algorithms", [](auto &_) {
  subsuite<>(_, "stringify()", [](auto &_) {
    using detail::stringify;

    _.test("std::string", []() {
      std::string s = "text";
      expect(stringify(s), equal_to("text"));
    });

    _.test("non-string", []() {
      int i = 123;
      expect(stringify(i), equal_to("123"));
    });
  });

  subsuite<std::vector<int>>(_, "joined()", int_factory{}, [](auto &_) {
    using detail::joined;

    _.test("joined(x)", [](auto &container) {
      std::ostringstream ss1;
      ss1 << joined(container);
      expect(ss1.str(), equal_to("1, 2, 3"));

      std::ostringstream ss2;
      ss2 << joined({1, 2, 3});
      expect(ss2.str(), equal_to("1, 2, 3"));
    });

    _.test("joined(x, func)", [](auto &container) {
      std::ostringstream ss1;
      ss1 << joined(container, [](auto &&i) { return 2 * i; });
      expect(ss1.str(), equal_to("2, 4, 6"));

      std::ostringstream ss2;
      ss2 << joined({1, 2, 3}, [](auto &&i) { return 2 * i; });
      expect(ss2.str(), equal_to("2, 4, 6"));
    });

    _.test("joined(x, func, delim)", [](auto &container) {
      std::ostringstream ss1;
      ss1 << joined(container, [](auto &&i) { return 2 * i; }, " and ");
      expect(ss1.str(), equal_to("2 and 4 and 6"));

      std::ostringstream ss2;
      ss2 << joined({1, 2, 3}, [](auto &&i) { return 2 * i; }, " and ");
      expect(ss2.str(), equal_to("2 and 4 and 6"));
    });

  });

  subsuite<std::vector<int>>(_, "iter_joined()", int_factory{}, [](auto &_) {
    using detail::iter_joined;

    _.test("iter_joined(begin, end)", [](auto &container) {
      std::ostringstream ss;
      ss << iter_joined(container.begin(), container.end());
      expect(ss.str(), equal_to("1, 2, 3"));
    });

    _.test("iter_joined(begin, end, func)", [](auto &container) {
      std::ostringstream ss;
      ss << iter_joined(container.begin(), container.end(),
                   [](auto &&i) { return 2 * i; });
      expect(ss.str(), equal_to("2, 4, 6"));
    });

    _.test("iter_joined(begin, end, func, delim)", [](auto &container) {
      std::ostringstream ss;
      ss << iter_joined(container.begin(), container.end(),
                   [](auto &&i) { return 2 * i; }, " and ");
      expect(ss.str(), equal_to("2 and 4 and 6"));
    });
  });
});

#endif
