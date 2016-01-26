#include <mettle.hpp>
using namespace mettle;

struct int_factory {
  template<typename T>
  T make() const {
    return {1, 2, 3};
  }
};

// XXX: Enable these tests on MSVC (they trigger a stdlib error).
#if !defined(_MSC_VER) || defined(__clang__)

suite<> test_string_alg("string algorithms", [](auto &_) {
  subsuite<>(_, "stringify()", [](auto &_) {
    _.test("std::string", []() {
      std::string s = "text";
      expect(detail::stringify(s), equal_to("text"));
    });

    _.test("non-string", []() {
      int i = 123;
      expect(detail::stringify(i), equal_to("123"));
    });
  });

  subsuite<std::vector<int>>(_, "joined()", int_factory{}, [](auto &_) {
    _.test("joined(x)", [](auto &container) {
      std::ostringstream ss1;
      ss1 << detail::joined(container);
      expect(ss1.str(), equal_to("1, 2, 3"));

      std::ostringstream ss2;
      ss2 << detail::joined({1, 2, 3});
      expect(ss2.str(), equal_to("1, 2, 3"));
    });

    _.test("joined(x, func)", [](auto &container) {
      std::ostringstream ss1;
      ss1 << detail::joined(container, [](int i) { return 2 * i; });
      expect(ss1.str(), equal_to("2, 4, 6"));

      std::ostringstream ss2;
      ss2 << detail::joined({1, 2, 3}, [](int i) { return 2 * i; });
      expect(ss2.str(), equal_to("2, 4, 6"));
    });

    _.test("joined(x, func, delim)", [](auto &container) {
      std::ostringstream ss1;
      ss1 << detail::joined(container, [](int i) { return 2 * i; }, " and ");
      expect(ss1.str(), equal_to("2 and 4 and 6"));

      std::ostringstream ss2;
      ss2 << detail::joined({1, 2, 3}, [](int i) { return 2 * i; }, " and ");
      expect(ss2.str(), equal_to("2 and 4 and 6"));
    });

  });

  subsuite<std::vector<int>>(_, "iter_joined()", int_factory{}, [](auto &_) {
    _.test("iter_joined(begin, end)", [](auto &container) {
      std::ostringstream ss;
      ss << detail::iter_joined(container.begin(), container.end());
      expect(ss.str(), equal_to("1, 2, 3"));
    });

    _.test("iter_joined(begin, end, func)", [](auto &container) {
      std::ostringstream ss;
      ss << detail::iter_joined(container.begin(), container.end(),
                                [](int i) { return 2 * i; });
      expect(ss.str(), equal_to("2, 4, 6"));
    });

    _.test("iter_joined(begin, end, func, delim)", [](auto &container) {
      std::ostringstream ss;
      ss << detail::iter_joined(container.begin(), container.end(),
                                [](int i) { return 2 * i; }, " and ");
      expect(ss.str(), equal_to("2 and 4 and 6"));
    });
  });
});

#endif
