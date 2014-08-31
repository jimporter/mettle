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

  template<typename Tuple>
  struct compiled_subsuite_helper;

  template<typename ...T>
  struct compiled_subsuite_helper<std::tuple<T...>> {
    using type = compiled_suite<void, T...>;
  };
}

template<typename Tuple>
using compiled_subsuite = typename detail::compiled_subsuite_helper<Tuple>
  ::type;

constexpr detail::auto_factory_t auto_factory;
constexpr detail::type_only_factory_t type_only;

template<typename Factory, typename ParentFixture, typename ...Fixture>
class subsuite_builder;

template<typename ParentFixture, typename ...Fixture, typename Factory,
         typename F>
compiled_subsuite<ParentFixture>
make_subsuite(const std::string &name, const attributes &attrs,
              Factory &&factory, const F &f);

template<typename ParentFixture, typename ...Fixture, typename Factory,
         typename F>
inline compiled_subsuite<ParentFixture>
make_subsuite(const std::string &name, Factory &&factory, const F &f) {
  return make_subsuite<ParentFixture, Fixture...>(
    name, {}, std::forward<Factory>(factory), f
  );
}

template<typename ParentFixture, typename ...Fixture, typename F>
inline compiled_subsuite<ParentFixture>
make_subsuite(const std::string &name, const attributes &attrs, const F &f) {
  return make_subsuite<ParentFixture, Fixture...>(name, attrs, auto_factory, f);
}

template<typename ParentFixture, typename ...Fixture, typename F>
inline compiled_subsuite<ParentFixture>
make_subsuite(const std::string &name, const F &f) {
  return make_subsuite<ParentFixture, Fixture...>(name, auto_factory, f);
}


template<typename ParentFixture, typename Factory, typename F>
std::array<compiled_subsuite<ParentFixture>, 1>
make_subsuites(const std::string &name, const attributes &attrs,
               Factory &&factory, const F &f) {
  return {{ make_subsuite<ParentFixture>(
    name, attrs, std::forward<Factory>(factory), f
  ) }};
}

template<typename ParentFixture, typename Fixture, typename Factory, typename F>
std::array<compiled_subsuite<ParentFixture>, 1>
make_subsuites(const std::string &name, const attributes &attrs,
               Factory &&factory, const F &f) {
  return {{ make_subsuite<ParentFixture, Fixture>(
    name, attrs, std::forward<Factory>(factory), f
  ) }};
}

template<typename ParentFixture, typename First, typename Second,
         typename ...Rest, typename Factory, typename F>
std::array<compiled_subsuite<ParentFixture>, sizeof...(Rest) + 2>
make_subsuites(const std::string &name, const attributes &attrs,
               Factory &&factory, const F &f) {
  using detail::annotate_type;
  return {{
    make_subsuite<ParentFixture, First>(
      annotate_type<First>(name), attrs, factory, f
    ),
    make_subsuite<ParentFixture, Second>(
      annotate_type<Second>(name), attrs, factory, f
    ),
    make_subsuite<ParentFixture, Rest>(
      annotate_type<Rest>(name), attrs, factory, f
    )...
  }};
}

template<typename ParentFixture, typename ...Fixture, typename Factory,
         typename F>
inline std::array<compiled_subsuite<ParentFixture>,
                  std::max<size_t>(sizeof...(Fixture), 1)>
make_subsuites(const std::string &name, Factory &&factory, const F &f) {
  return make_subsuites<ParentFixture, Fixture...>(
    name, {}, std::forward<Factory>(factory), f
  );
}

template<typename ParentFixture, typename ...Fixture, typename F>
inline std::array<compiled_subsuite<ParentFixture>,
                  std::max<size_t>(sizeof...(Fixture), 1)>
make_subsuites(const std::string &name, const attributes &attrs, const F &f) {
  return make_subsuites<ParentFixture, Fixture...>(
    name, attrs, auto_factory, f
  );
}

template<typename ParentFixture, typename ...Fixture, typename F>
inline std::array<compiled_subsuite<ParentFixture>,
                  std::max<size_t>(sizeof...(Fixture), 1)>
make_subsuites(const std::string &name, const F &f) {
  return make_subsuites<ParentFixture, Fixture...>(name, auto_factory, f);
}

template<typename ...T>
class suite_builder_base {
public:
  using tuple_type = std::tuple<T...>;
  using function_type = std::function<void(T&...)>;

  suite_builder_base(std::string name, attributes attrs)
    : name_(std::move(name)), attrs_(std::move(attrs)) {}
  suite_builder_base(const suite_builder_base &) = delete;
  suite_builder_base & operator =(const suite_builder_base &) = delete;

  void setup(function_type f) {
    setup_ = std::move(f);
  }

  void teardown(function_type f) {
    teardown_ = std::move(f);
  }

  void test(std::string name, function_type f) {
    tests_.push_back({std::move(name), std::move(f), {}});
  }

  void test(std::string name, attributes attrs, function_type &&f) {
    tests_.push_back({
      std::move(name), std::move(f), std::move(attrs)
    });
  }

  void subsuite(compiled_suite<void, T...> subsuite) {
    subsuites_.push_back(std::move(subsuite));
  }

  template<typename Subsuites>
  void subsuite(Subsuites &&subsuites) {
    for(auto &&i : subsuites)
      subsuites_.push_back(detail::move_if<Subsuites>(i));
  }

  template<typename ...Fixture, typename ...Args>
  void subsuite(const std::string &name, const attributes &attrs,
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
    attributes attrs;
  };

  std::string name_;
  attributes attrs_;
  function_type setup_, teardown_;
  std::vector<test_info> tests_;
  std::vector<compiled_suite<void, T...>> subsuites_;
};

namespace detail {
  template<typename ParentFixture, typename OutChild>
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

template<typename Factory, typename ParentFixture, typename ...Fixture>
class subsuite_builder
  : public suite_builder_base_t<Factory, ParentFixture, Fixture...> {
private:
  static_assert(sizeof...(Fixture) < 2, "only specify one fixture at a time!");
  using base = suite_builder_base_t<Factory, ParentFixture, Fixture...>;
public:
  using factory_type = Factory;
  using fixture_type = detail::first_t<Fixture...>;
  using compiled_suite_type = compiled_subsuite<ParentFixture>;

  subsuite_builder(const std::string &name, const attributes &attrs,
                   Factory factory)
    : base(name, attrs), factory_(factory) {}

  compiled_suite_type finalize() {
    return compiled_suite_type(
      std::move(base::name_), std::move(base::tests_),
      std::move(base::subsuites_), std::move(base::attrs_),
      std::bind(&subsuite_builder::wrap_test, this, std::placeholders::_1)
    );
  }
private:
  auto wrap_test(typename base::function_type test) {
    return detail::test_caller<factory_type, ParentFixture, Fixture...>{
      factory_, base::setup_, base::teardown_, std::move(test)
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

  suite_builder(const std::string &name, const attributes &attrs,
                Factory factory)
    : base(name, attrs), factory_(factory) {}

  runnable_suite finalize() {
    return runnable_suite(
      std::move(base::name_), std::move(base::tests_),
      std::move(base::subsuites_), std::move(base::attrs_),
      std::bind(&suite_builder::wrap_test, this, std::placeholders::_1)
    );
  }
private:
  auto wrap_test(typename base::function_type test) {
    return [
      test_function = detail::test_caller<factory_type, std::tuple<>, T...>{
        factory_, base::setup_, base::teardown_, std::move(test)
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
  }

  factory_type factory_;
};

template<typename Exception, typename ...Fixture, typename Factory, typename F>
runnable_suite
make_basic_suite(const std::string &name, const attributes &attrs,
                 Factory &&factory, const F &f) {
  using FactoryValue = typename std::remove_reference<Factory>::type;
  suite_builder<Exception, FactoryValue, Fixture...> builder(
    name, attrs, std::forward<Factory>(factory)
  );
  f(builder);
  return builder.finalize();
}

template<typename Exception, typename ...Fixture, typename Factory, typename F>
inline runnable_suite
make_basic_suite(const std::string &name, Factory &&factory, const F &f) {
  return make_basic_suite<Exception, Fixture...>(
    name, {}, std::forward<Factory>(factory), f
  );
}

template<typename Exception, typename ...Fixture, typename F>
inline runnable_suite
make_basic_suite(const std::string &name, const attributes &attrs, const F &f) {
  return make_basic_suite<Exception, Fixture...>(name, attrs, auto_factory, f);
}

template<typename Exception, typename ...Fixture, typename F>
inline runnable_suite
make_basic_suite(const std::string &name, const F &f) {
  return make_basic_suite<Exception, Fixture...>(name, auto_factory, f);
}


template<typename Exception, typename Factory, typename F>
std::array<runnable_suite, 1>
make_basic_suites(const std::string &name, const attributes &attrs,
                  Factory &&factory, const F &f) {
  return {{ make_basic_suite<Exception>(
    name, attrs, std::forward<Factory>(factory), f
  ) }};
}

template<typename Exception, typename Fixture, typename Factory, typename F>
std::array<runnable_suite, 1>
make_basic_suites(const std::string &name, const attributes &attrs,
                  Factory &&factory, const F &f) {
  return {{ make_basic_suite<Exception, Fixture>(
    name, attrs, std::forward<Factory>(factory), f
  ) }};
}

template<typename Exception, typename First, typename Second,
         typename ...Rest, typename Factory, typename F>
std::array<runnable_suite, sizeof...(Rest) + 2>
make_basic_suites(const std::string &name, const attributes &attrs,
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
inline std::array<runnable_suite, std::max<size_t>(sizeof...(Fixture), 1)>
make_basic_suites(const std::string &name, Factory &&factory, const F &f) {
  return make_basic_suites<Exception, Fixture...>(
    name, {}, std::forward<Factory>(factory), f
  );
}

template<typename Exception, typename ...Fixture, typename F>
inline std::array<runnable_suite, std::max<size_t>(sizeof...(Fixture), 1)>
make_basic_suites(const std::string &name, const attributes &attrs,
                  const F &f) {
  return make_basic_suites<Exception, Fixture...>(name, attrs, auto_factory, f);
}

template<typename Exception, typename ...Fixture, typename F>
inline std::array<runnable_suite, std::max<size_t>(sizeof...(Fixture), 1)>
make_basic_suites(const std::string &name, const F &f) {
  return make_basic_suites<Exception, Fixture...>(name, auto_factory, f);
}


template<typename ParentFixture, typename ...Fixture, typename Factory,
         typename F>
compiled_subsuite<ParentFixture>
make_subsuite(const std::string &name, const attributes &attrs,
              Factory &&factory, const F &f) {
  using FactoryValue = typename std::remove_reference<Factory>::type;
  subsuite_builder<FactoryValue, ParentFixture, Fixture...> builder(
    name, attrs, std::forward<Factory>(factory)
  );
  f(builder);
  return builder.finalize();
}


template<typename ...Fixture, typename Parent, typename ...Args>
inline compiled_subsuite<typename Parent::tuple_type>
make_subsuite(const Parent &, const std::string &name, const attributes &attrs,
              Args &&...args) {
  return make_subsuite<typename Parent::tuple_type, Fixture...>(
    name, attrs, std::forward<Args>(args)...
  );
}

template<typename ...Fixture, typename Parent, typename ...Args>
inline compiled_subsuite<typename Parent::tuple_type>
make_subsuite(const Parent &, const std::string &name, Args &&...args) {
  return make_subsuite<typename Parent::tuple_type, Fixture...>(
    name, std::forward<Args>(args)...
  );
}


template<typename ...Fixture, typename Parent, typename ...Args>
inline std::array<compiled_subsuite<typename Parent::tuple_type>,
                  std::max<size_t>(sizeof...(Fixture), 1)>
make_subsuites(const Parent &, const std::string &name, const attributes &attrs,
               Args &&...args) {
  return make_subsuites<typename Parent::tuple_type, Fixture...>(
    name, attrs, std::forward<Args>(args)...
  );
}

template<typename ...Fixture, typename Parent, typename ...Args>
inline std::array<compiled_subsuite<typename Parent::tuple_type>,
                  std::max<size_t>(sizeof...(Fixture), 1)>
make_subsuites(const Parent &, const std::string &name, Args &&...args) {
  return make_subsuites<typename Parent::tuple_type, Fixture...>(
    name, std::forward<Args>(args)...
  );
}


template<typename ...Fixture, typename Parent, typename ...Args>
inline void
subsuite(Parent &builder, const std::string &name, const attributes &attrs,
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
