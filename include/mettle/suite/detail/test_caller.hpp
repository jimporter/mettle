#ifndef INC_METTLE_SUITE_DETAIL_TEST_CALLER_HPP
#define INC_METTLE_SUITE_DETAIL_TEST_CALLER_HPP

#include <functional>
#include <tuple>
#include <utility>

#include "../factory.hpp"

namespace mettle::detail {

  template<typename ...Args>
  struct test_caller {
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

  template<typename Factory, typename Child, typename ...Parent>
  requires factory_for<Factory, Child>
  struct fixture_test_caller : test_caller<
    Parent..., factory_result_t<Factory, Child>
  > {
    inline void operator ()(Parent &...args) {
      using base = test_caller<Parent..., factory_result_t<Factory, Child>>;
      auto &&child = factory.template make<Child>();
      base::operator ()(args..., child);
    }

    Factory factory;
  };

} // namespace mettle::detail

#endif
