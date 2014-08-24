#ifndef INC_METTLE_SUITE_HPP
#define INC_METTLE_SUITE_HPP

#include <algorithm>
#include <array>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "attributes.hpp"
#include "compiled_suite.hpp"
#include "type_name.hpp"

namespace mettle {

namespace detail {
  template<typename ...>
  struct first;

  template<typename First, typename ...Rest>
  struct first<First, Rest...> {
    using type = First;
  };

  template<>
  struct first<> {
    using type = void;
  };

  template<typename ...T>
  using first_t = typename first<T...>::type;

  template<typename Factory, typename ...Child>
  struct transform_fixture;

  template<typename Factory>
  struct transform_fixture<Factory> {
    using type = void;
  };

  template<typename Factory, typename Child>
  struct transform_fixture<Factory, Child> {
    using type = decltype(std::declval<Factory>().template make<Child>());
  };

  template<typename Factory, typename ...Child>
  using transform_fixture_t = typename transform_fixture<
    Factory, Child...
  >::type;

  template<typename Function>
  struct test_caller_sub_base {
    template<typename ...Args>
    void call_test(Args &...args) {
      if(setup)
        setup(args...);

      try {
        test(args...);
      }
      catch(...) {
        if(teardown)
          teardown(args...);
        throw;
      }

      if(teardown)
        teardown(args...);
    }

    std::function<Function> setup, teardown, test;
  };

  template<typename Factory, typename Parent, typename InChild,
           typename OutChild>
  class test_caller_base;

  template<typename Factory, typename ...Parent, typename InChild,
           typename OutChild>
  class test_caller_base<Factory, std::tuple<Parent...>, InChild, OutChild>
    : private test_caller_sub_base<void(Parent&..., OutChild&)> {
  private:
    using base = test_caller_sub_base<void(Parent&..., OutChild&)>;
  public:
    template<typename ...T>
    test_caller_base(Factory f, T &&...t)
      : base{std::forward<T>(t)...}, factory(f) {}

    inline void operator ()(Parent &...args) {
      auto &&child = factory.template make<InChild>();
      base::call_test(args..., child);
    }

    Factory factory;
  };

  template<typename Factory, typename ...Parent, typename InChild>
  class test_caller_base<Factory, std::tuple<Parent...>, InChild, void>
    : private test_caller_sub_base<void(Parent&...)> {
  private:
    using base = test_caller_sub_base<void(Parent&...)>;
  public:
    template<typename ...T>
    test_caller_base(const Factory &, T &&...t)
      : base{std::forward<T>(t)...} {}
  public:
    inline void operator ()(Parent &...args) {
      base::call_test(args...);
    }
  };

  template<typename Factory, typename Parent, typename ...Child>
  class test_caller : public test_caller_base<
    Factory, Parent, first_t<Child...>, transform_fixture_t<Factory, Child...>
  > {
  private:
    using base = test_caller_base<
      Factory, Parent, first_t<Child...>, transform_fixture_t<Factory, Child...>
    >;
  public:
    using base::base;
  };

  template<typename T>
  std::string annotate_type(const std::string &s) {
    return s + " (" + type_name<T>() + ")";
  }

  struct auto_factory_t {
    constexpr auto_factory_t() {}

    template<typename T>
    T make() const {
      return {};
    }
  };

  struct type_only_factory_t {
    constexpr type_only_factory_t() {}

    template<typename T>
    void make() const {}
  };
}

constexpr detail::auto_factory_t auto_factory;
constexpr detail::type_only_factory_t type_only;

template<typename Factory, typename Parent, typename ...Fixture>
class subsuite_builder;

template<typename Parent, typename ...Fixture, typename Factory, typename F>
typename subsuite_builder<Factory, Parent, Fixture...>::compiled_suite_type
make_subsuite(const std::string &name, const attr_list &attrs,
              Factory &&factory, const F &f);

template<typename Parent, typename ...Fixture, typename Factory, typename F>
inline auto
make_subsuite(const std::string &name, Factory &&factory, const F &f) {
  return make_subsuite<Parent, Fixture...>(
    name, {}, std::forward<Factory>(factory), f
  );
}

template<typename Parent, typename ...Fixture, typename F>
inline auto
make_subsuite(const std::string &name, const attr_list &attrs, const F &f) {
  return make_subsuite<Parent, Fixture...>(name, attrs, auto_factory, f);
}

template<typename Parent, typename ...Fixture, typename F>
inline auto
make_subsuite(const std::string &name, const F &f) {
  return make_subsuite<Parent, Fixture...>(name, auto_factory, f);
}


template<typename Parent, typename Factory, typename F>
std::array<typename subsuite_builder<Factory, Parent>::compiled_suite_type, 1>
make_subsuites(const std::string &name, const attr_list &attrs,
               Factory &&factory, const F &f) {
  return {{ make_subsuite<Parent>(
    name, attrs, std::forward<Factory>(factory), f
  ) }};
}

template<typename Parent, typename Fixture, typename Factory, typename F>
std::array<typename subsuite_builder<Factory, Parent>::compiled_suite_type, 1>
make_subsuites(const std::string &name, const attr_list &attrs,
               Factory &&factory, const F &f) {
  return {{ make_subsuite<Parent, Fixture>(
    name, attrs, std::forward<Factory>(factory), f
  ) }};
}

template<typename Parent, typename First, typename Second, typename ...Rest,
         typename Factory, typename F>
std::array<
  typename subsuite_builder<Factory, Parent, First>::compiled_suite_type,
  sizeof...(Rest) + 2
>
make_subsuites(const std::string &name, const attr_list &attrs,
               Factory &&factory, const F &f) {
  using detail::annotate_type;
  return {{
    make_subsuite<Parent, First>(
      annotate_type<First>(name), attrs, factory, f
    ),
    make_subsuite<Parent, Second>(
      annotate_type<Second>(name), attrs, factory, f
    ),
    make_subsuite<Parent, Rest>(
      annotate_type<Rest>(name), attrs, factory, f
    )...
  }};
}

template<typename Parent, typename ...Fixture, typename Factory, typename F>
inline auto
make_subsuites(const std::string &name, Factory &&factory, const F &f) {
  return make_subsuites<Parent, Fixture...>(
    name, {}, std::forward<Factory>(factory), f
  );
}

template<typename Parent, typename ...Fixture, typename F>
inline auto
make_subsuites(const std::string &name, const attr_list &attrs, const F &f) {
  return make_subsuites<Parent, Fixture...>(name, attrs, auto_factory, f);
}

template<typename Parent, typename ...Fixture, typename F>
inline auto
make_subsuites(const std::string &name, const F &f) {
  return make_subsuites<Parent, Fixture...>(name, auto_factory, f);
}

template<typename ...T>
class suite_builder_base {
public:
  using tuple_type = std::tuple<T...>;
  using function_type = std::function<void(T&...)>;

  suite_builder_base(const std::string &name, const attr_list &attrs)
    : name_(name), attrs_(attrs) {}
  suite_builder_base(const suite_builder_base &) = delete;
  suite_builder_base & operator =(const suite_builder_base &) = delete;

  void setup(const function_type &f) {
    setup_ = f;
  }

  void teardown(const function_type &f) {
    teardown_ = f;
  }

  void test(const std::string &name, const function_type &f) {
    tests_.push_back({name, f, {}});
  }

  void test(const std::string &name, const attr_list &attrs,
            const function_type &f) {
    tests_.push_back({name, f, attrs});
  }

  void subsuite(const compiled_suite<void, T...> &subsuite) {
    subsuites_.push_back(subsuite);
  }

  void subsuite(compiled_suite<void, T...> &&subsuite) {
    subsuites_.push_back(std::move(subsuite));
  }

  template<typename U>
  void subsuite(const U &subsuites) {
    for(const auto &i : subsuites)
      subsuites_.push_back(i);
  }

  template<typename U>
  void subsuite(U &&subsuites) {
    for(const auto &i : subsuites)
      subsuites_.push_back(std::move(i));
  }

  template<typename ...Fixture, typename ...Args>
  void subsuite(const std::string &name, const attr_list &attrs,
                Args &&...args) {
    subsuite(make_subsuites<tuple_type, Fixture...>(
      name, attrs, std::forward<Args>(args)...
    ));
  }

  template<typename ...Fixture, typename ...Args>
  void subsuite(const std::string &name, Args &&...args) {
    subsuite(make_subsuites<tuple_type, Fixture...>(
      name, std::forward<Args>(args)...
    ));
  }
protected:
  struct test_info {
    std::string name;
    function_type function;
    attr_list attrs;
  };

  std::string name_;
  attr_list attrs_;
  function_type setup_, teardown_;
  std::vector<test_info> tests_;
  std::vector<compiled_suite<void, T...>> subsuites_;
};

namespace detail {
  template<typename Parent, typename OutChild>
  struct suite_builder_base_helper;

  template<typename ...Parent, typename OutChild>
  struct suite_builder_base_helper<std::tuple<Parent...>, OutChild> {
    using type = suite_builder_base<Parent..., OutChild>;
  };

  template<typename ...Parent>
  struct suite_builder_base_helper<std::tuple<Parent...>, void> {
    using type = suite_builder_base<Parent...>;
  };
}

template<typename Factory, typename Parent, typename ...InChild>
struct suite_builder_base_type {
  using type = typename detail::suite_builder_base_helper<
    Parent, detail::transform_fixture_t<Factory, InChild...>
  >::type;
};

template<typename Factory, typename Parent, typename ...InChild>
using suite_builder_base_t = typename suite_builder_base_type<
  Factory, Parent, InChild...
>::type;

template<typename Factory, typename ...T, typename ...U>
class subsuite_builder<Factory, std::tuple<T...>, U...>
  : public suite_builder_base_t<Factory, std::tuple<T...>, U...> {
private:
  static_assert(sizeof...(U) < 2, "only specify one fixture at a time!");
  using base = suite_builder_base_t<Factory, std::tuple<T...>, U...>;
public:
  using factory_type = Factory;
  using fixture_type = detail::first_t<U...>;
  using compiled_suite_type = compiled_suite<void, T...>;

  subsuite_builder(const std::string &name, const attr_list &attrs,
                   Factory factory)
    : base(name, attrs), factory_(factory) {}

  compiled_suite_type finalize() {
    return compiled_suite_type(
      base::name_, base::tests_, base::subsuites_,
      [this](const auto &a) { return wrap_test(a); }
    );
  }
private:
  template<typename Test>
  auto wrap_test(const Test &test) {
    detail::test_caller<factory_type, std::tuple<T...>, U...> test_function{
      factory_, base::setup_, base::teardown_, test.function
    };

    return typename compiled_suite_type::test_info{
      test.name, test_function, unite(test.attrs, base::attrs_)
    };
  }

  factory_type factory_;
};

template<typename Exception, typename Factory, typename ...T>
class suite_builder : public suite_builder_base_t<Factory, std::tuple<>, T...> {
private:
  static_assert(sizeof...(T) < 2, "only specify one fixture at a time!");
  using base = suite_builder_base_t<Factory, std::tuple<>, T...>;
public:
  using exception_type = Exception;
  using factory_type = Factory;
  using fixture_type = detail::first_t<T...>;

  suite_builder(const std::string &name, const attr_list &attrs,
                Factory factory)
    : base(name, attrs), factory_(factory) {}

  runnable_suite finalize() {
    return runnable_suite(
      base::name_, base::tests_, base::subsuites_,
      [this](const auto &a) { return wrap_test(a); }
    );
  }
private:
  template<typename Test>
  auto wrap_test(const Test &test) {
    auto wrapped_test = [
      test_function = detail::test_caller<factory_type, std::tuple<>, T...>{
        factory_, base::setup_, base::teardown_, test.function
      }
    ]() mutable -> test_result {
      bool passed = false;
      std::string message;

      try {
        test_function();
        passed = true;
      }
      catch(const exception_type &e) {
        message = e.what();
      }
      catch(const std::exception &e) {
        message = std::string("Uncaught exception: ") + e.what();
      }
      catch(...) {
        message = "Unknown exception";
      }

      return { passed, message };
    };

    return runnable_suite::test_info{
      test.name, wrapped_test, unite(test.attrs, base::attrs_)
    };
  }

  factory_type factory_;
};

template<typename Exception, typename ...Fixture, typename Factory, typename F>
auto
make_basic_suite(const std::string &name, const attr_list &attrs,
                 Factory &&factory, const F &f) {
  suite_builder<Exception, Factory, Fixture...> builder(
    name, attrs, std::forward<Factory>(factory)
  );
  f(builder);
  return builder.finalize();
}

template<typename Exception, typename ...Fixture, typename Factory, typename F>
inline auto
make_basic_suite(const std::string &name, Factory &&factory, const F &f) {
  return make_basic_suite<Exception, Fixture...>(
    name, {}, std::forward<Factory>(factory), f
  );
}

template<typename Exception, typename ...Fixture, typename F>
inline auto
make_basic_suite(const std::string &name, const attr_list &attrs, const F &f) {
  return make_basic_suite<Exception, Fixture...>(name, attrs, auto_factory, f);
}

template<typename Exception, typename ...Fixture, typename F>
inline auto
make_basic_suite(const std::string &name, const F &f) {
  return make_basic_suite<Exception, Fixture...>(name, auto_factory, f);
}


template<typename Exception, typename Factory, typename F>
std::array<runnable_suite, 1>
make_basic_suites(const std::string &name, const attr_list &attrs,
                  Factory &&factory, const F &f) {
  return {{ make_basic_suite<Exception>(
    name, attrs, std::forward<Factory>(factory), f
  ) }};
}

template<typename Exception, typename Fixture, typename Factory, typename F>
std::array<runnable_suite, 1>
make_basic_suites(const std::string &name, const attr_list &attrs,
                  Factory &&factory, const F &f) {
  return {{ make_basic_suite<Exception, Fixture>(
    name, attrs, std::forward<Factory>(factory), f
  ) }};
}

template<typename Exception, typename First, typename Second,
         typename ...Rest, typename Factory, typename F>
std::array<runnable_suite, sizeof...(Rest) + 2>
make_basic_suites(const std::string &name, const attr_list &attrs,
                  Factory &&factory, const F &f) {
  using detail::annotate_type;
  return {{
    make_basic_suite<Exception, First>(
      annotate_type<First>(name), attrs, factory, f
    ),
    make_basic_suite<Exception, Second>(
      annotate_type<Second>(name), attrs, factory, f
    ),
    make_basic_suite<Exception, Rest>(
      annotate_type<Rest>(name), attrs, factory, f
    )...
  }};
}

template<typename Exception, typename ...Fixture, typename Factory, typename F>
inline auto
make_basic_suites(const std::string &name, Factory &&factory, const F &f) {
  return make_basic_suites<Exception, Fixture...>(
    name, {}, std::forward<Factory>(factory), f
  );
}

template<typename Exception, typename ...Fixture, typename F>
inline auto
make_basic_suites(const std::string &name, const attr_list &attrs, const F &f) {
  return make_basic_suites<Exception, Fixture...>(name, attrs, auto_factory, f);
}

template<typename Exception, typename ...Fixture, typename F>
inline auto
make_basic_suites(const std::string &name, const F &f) {
  return make_basic_suites<Exception, Fixture...>(name, auto_factory, f);
}


template<typename Parent, typename ...Fixture, typename Factory, typename F>
typename subsuite_builder<Factory, Parent, Fixture...>::compiled_suite_type
make_subsuite(const std::string &name, const attr_list &attrs,
              Factory &&factory, const F &f) {
  subsuite_builder<Factory, Parent, Fixture...> builder(
    name, attrs, std::forward<Factory>(factory)
  );
  f(builder);
  return builder.finalize();
}


template<typename ...Fixture, typename Parent, typename ...Args>
inline auto
make_subsuite(const Parent &, const std::string &name, const attr_list &attrs,
              Args &&...args) {
  return make_subsuite<typename Parent::tuple_type, Fixture...>(
    name, attrs, std::forward<Args>(args)...
  );
}

template<typename ...Fixture, typename Parent, typename ...Args>
inline auto
make_subsuite(const Parent &, const std::string &name, Args &&...args) {
  return make_subsuite<typename Parent::tuple_type, Fixture...>(
    name, std::forward<Args>(args)...
  );
}


template<typename ...Fixture, typename Parent, typename ...Args>
inline auto
make_subsuites(const Parent &, const std::string &name, const attr_list &attrs,
               Args &&...args) {
  return make_subsuites<typename Parent::tuple_type, Fixture...>(
    name, attrs, std::forward<Args>(args)...
  );
}

template<typename ...Fixture, typename Parent, typename ...Args>
inline auto
make_subsuites(const Parent &, const std::string &name, Args &&...args) {
  return make_subsuites<typename Parent::tuple_type, Fixture...>(
    name, std::forward<Args>(args)...
  );
}


template<typename ...Fixture, typename Parent, typename ...Args>
inline void
subsuite(Parent &builder, const std::string &name, const attr_list &attrs,
         Args &&...args) {
  builder.template subsuite<Fixture...>(
    name, attrs, std::forward<Args>(args)...
  );
}

template<typename ...Fixture, typename Parent, typename ...Args>
inline void
subsuite(Parent &builder, const std::string &name, Args &&...args) {
  builder.template subsuite<Fixture...>(name, std::forward<Args>(args)...);
}


template<typename T>
struct fixture_type {
  using type = typename std::remove_reference_t<T>::fixture_type;
};

template<typename T>
using fixture_type_t = typename fixture_type<T>::type;

} // namespace mettle

#endif
