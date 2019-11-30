#include <mettle.hpp>
using namespace mettle;

#include "run_counter.hpp"

template<typename Tuple>
struct run_counter_from_tuple_t;

template<typename ...T>
struct run_counter_from_tuple_t<std::tuple<T...>> {
  using type = run_counter<T &...>;
};

template<typename Tuple>
using run_counter_from_tuple = typename run_counter_from_tuple_t<Tuple>::type;

using namespace mettle::detail;

suite<std::tuple<>, std::tuple<int, int>>
test_test_caller("test_caller", [](auto &_) {
  using Fixture = fixture_type_t<decltype(_)>;

  _.test("test with no fixture", [](auto &tup) {
    run_counter_from_tuple<Fixture> setup, teardown, test;
    test_caller<auto_factory_t, Fixture> t(
      auto_factory, setup, teardown, test
    );
    std::apply(t, tup);

    expect("setup run count", setup.runs(), equal_to(1));
    expect("test run count", test.runs(), equal_to(1));
    expect("teardown run count", teardown.runs(), equal_to(1));
  });

  _.test("test with auto fixture", [](auto &tup) {
    run_counter_from_tuple<decltype(
      std::tuple_cat(tup, std::tuple<int>())
    )> setup, teardown, test;
    test_caller<auto_factory_t, Fixture, int> t(
      auto_factory, setup, teardown, test
    );
    std::apply(t, tup);

    expect("setup run count", setup.runs(), equal_to(1));
    expect("test run count", test.runs(), equal_to(1));
    expect("teardown run count", teardown.runs(), equal_to(1));
  });

  _.test("test with type-only fixture", [](auto &tup) {
    run_counter_from_tuple<Fixture> setup, teardown, test;
    test_caller<type_only_factory_t, Fixture, int> t(
      type_only, setup, teardown, test
    );
    std::apply(t, tup);

    expect("setup run count", setup.runs(), equal_to(1));
    expect("test run count", test.runs(), equal_to(1));
    expect("teardown run count", teardown.runs(), equal_to(1));
  });
});
