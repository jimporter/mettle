#include <mettle.hpp>
using namespace mettle;

#include "run_counter.hpp"

template<template<typename ...> typename T, typename Tuple,
         template<typename ...> typename Transform>
struct apply_tuple_t;

template<template<typename ...> typename T, typename ...Args,
         template<typename ...> typename Transform>
struct apply_tuple_t<T, std::tuple<Args...>, Transform> {
  using type = T<Transform<Args>...>;
};

template<template<typename ...> typename T, typename Tuple,
         template<typename ...> typename Transform = std::type_identity_t>
using apply_tuple = typename apply_tuple_t<T, Tuple, Transform>::type;

template<typename Tuple>
using run_counter_from_tuple = apply_tuple<
  run_counter, Tuple, std::add_lvalue_reference_t
>;


using namespace mettle::detail;

suite<std::tuple<>, std::tuple<int, int>>
test_test_caller("test_caller", [](auto &_) {
  using Fixture = fixture_type_t<decltype(_)>;

  _.test("no fixture", [](auto &tup) {
    run_counter_from_tuple<Fixture> setup, teardown, test;
    apply_tuple<test_caller, Fixture> caller(
      auto_factory, setup, teardown, test
    );
    std::apply(caller, tup);

    expect("setup run count", setup.runs(), equal_to(1));
    expect("test run count", test.runs(), equal_to(1));
    expect("teardown run count", teardown.runs(), equal_to(1));
  });

  _.test("fixture", [](auto &tup) {
    run_counter_from_tuple<decltype(
      std::tuple_cat(tup, std::tuple<int>())
    )> setup, teardown, test;
    apply_tuple<fixture_test_caller, decltype(
      std::tuple_cat(std::tuple<auto_factory_t, int>(), tup)
    )> caller(auto_factory, setup, teardown, test);
    std::apply(caller, tup);

    expect("setup run count", setup.runs(), equal_to(1));
    expect("test run count", test.runs(), equal_to(1));
    expect("teardown run count", teardown.runs(), equal_to(1));
  });
});
