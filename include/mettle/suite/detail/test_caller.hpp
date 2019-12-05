#ifndef INC_METTLE_SUITE_DETAIL_TEST_CALLER_HPP
#define INC_METTLE_SUITE_DETAIL_TEST_CALLER_HPP

#include <functional>
#include <tuple>
#include <utility>

namespace mettle::detail {

  struct no_fixture_t {};

  template<typename Factory, typename Child>
  struct transform_fixture {
    using type = decltype(std::declval<Factory>().template make<Child>());
  };

  template<typename Factory>
  struct transform_fixture<Factory, no_fixture_t> {
    using type = void;
  };

  template<typename Factory, typename Child>
  using transform_fixture_t = typename transform_fixture<
    Factory, Child
  >::type;

  template<typename ...Args>
  struct test_caller_base {
    using function_type = std::function<void(Args&...)>;

    void call_test(Args &...args) {
      if(setup)
        setup(args...);

      try {
        test(args...);
      } catch(...) {
        if(teardown) {
          try { teardown(args...); } catch(...) {}
        }
        throw;
      }

      if(teardown)
        teardown(args...);
    }

    function_type setup, teardown, test;
  };

  template<typename Factory, typename Parent, typename InChild,
           typename OutChild>
  class test_caller_impl;

  template<typename Factory, typename ...Parent, typename InChild,
           typename OutChild>
  class test_caller_impl<Factory, std::tuple<Parent...>, InChild, OutChild>
    : private test_caller_base<Parent &..., OutChild &> {
  private:
    using base = test_caller_base<Parent &..., OutChild &>;
  public:
    template<typename ...T>
    test_caller_impl(Factory f, T &&...t)
      : base{std::forward<T>(t)...}, factory(std::move(f)) {}

    inline void operator ()(Parent &...args) {
      auto &&child = factory.template make<InChild>();
      base::call_test(args..., child);
    }

    Factory factory;
  };

  template<typename Factory, typename ...Parent, typename InChild>
  class test_caller_impl<Factory, std::tuple<Parent...>, InChild, void>
    : private test_caller_base<Parent &...> {
  private:
    using base = test_caller_base<Parent &...>;
  public:
    template<typename ...T>
    test_caller_impl(const Factory &, T &&...t)
      : base{std::forward<T>(t)...} {}
  public:
    inline void operator ()(Parent &...args) {
      base::call_test(args...);
    }
  };

  template<typename Factory, typename Parent, typename Child>
  using test_caller = test_caller_impl<
    Factory, Parent, Child, transform_fixture_t<Factory, Child>
  >;

} // namespace mettle::detail

#endif
