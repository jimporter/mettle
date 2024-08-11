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

    void operator ()(Args &...args) {
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

  template<typename ...Parent>
  struct test_caller : test_caller_base<Parent &...> {
  private:
    using base = test_caller_base<Parent &...>;
  public:
    template<typename Factory, typename ...T>
    test_caller(const Factory &, T &&...t)
      : base{std::forward<T>(t)...} {}
  };

  template<typename Factory, typename Child, typename ...Parent>
  struct fixture_test_caller : test_caller_base<
    Parent &..., transform_fixture_t<Factory, Child> &
  > {
  private:
    using base = test_caller_base<
      Parent &..., transform_fixture_t<Factory, Child> &
    >;
  public:
    template<typename ...T>
    fixture_test_caller(Factory f, T &&...t)
      : base{std::forward<T>(t)...}, factory(std::move(f)) {}

    inline void operator ()(Parent &...args) {
      auto &&child = factory.template make<Child>();
      base::operator ()(args..., child);
    }

    Factory factory;
  };

} // namespace mettle::detail

#endif
