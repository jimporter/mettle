#include <mettle.hpp>
using namespace mettle;

#include <mettle/driver/object_factory.hpp>

auto equal_factory(std::string name, int result) {
  return make_matcher(
    std::move(name),
    [result](const auto &actual, const auto &name) -> match_result {
      auto actual_result = actual.second(1);
      std::ostringstream ss;
      ss << "[name = " << actual.first << ", f(1) = " << actual_result << "]";
      return { actual.first == name && actual_result == result, ss.str() };
    }, {"[name = ", ", f(1) = " + std::to_string(result) + "]"}
  );
}

suite<object_factory<int(int)>>
test_object_factory("object_factory", [](auto &_) {
  _.setup([](object_factory<int(int)> &f) {
    f.add("double", [](int i) {
      return 2*i;
    });

    f.add("triple", [](int i) {
      return 3*i;
    });
  });

  _.test("make()", [](object_factory<int(int)> &f) {
    auto x = f.make("double", 1);
    expect(x, equal_to(2));

    expect([&f]() {
      f.make("unknown", 1);
    }, thrown<std::out_of_range>());
  });

  _.test("iteration", [](object_factory<int(int)> &f) {
    expect(f, array( equal_factory("double", 2), equal_factory("triple", 3) ));
  });
});
