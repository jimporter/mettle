#include <mettle.hpp>
using namespace mettle;

#include "run_counter.hpp"

// Check if std::apply exists, and if not, use our own implementation.
#if !(__cplusplus >= 201703L || (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L))
template<typename F, typename Tuple, std::size_t ...I>
decltype(auto) apply_impl(F &&f, Tuple &&t, std::index_sequence<I...>) {
  return std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))...);
}

template<typename F, typename Tuple>
decltype(auto) apply(F&& f, Tuple &&t) {
  using Indices = std::make_index_sequence<
    std::tuple_size<std::decay_t<Tuple>>::value
  >;
  return apply_impl(std::forward<F>(f), std::forward<Tuple>(t), Indices());
}
#endif

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
    apply(t, tup);

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
    apply(t, tup);

    expect("setup run count", setup.runs(), equal_to(1));
    expect("test run count", test.runs(), equal_to(1));
    expect("teardown run count", teardown.runs(), equal_to(1));
  });

  _.test("test with type-only fixture", [](auto &tup) {
    run_counter_from_tuple<Fixture> setup, teardown, test;
    test_caller<type_only_factory_t, Fixture, int> t(
      type_only, setup, teardown, test
    );
    apply(t, tup);

    expect("setup run count", setup.runs(), equal_to(1));
    expect("test run count", test.runs(), equal_to(1));
    expect("teardown run count", teardown.runs(), equal_to(1));
  });
});
