#ifndef INC_METTLE_SUITE_MAKE_SUITE_HPP
#define INC_METTLE_SUITE_MAKE_SUITE_HPP

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>
#include <vector>

#include "attributes.hpp"
#include "compiled_suite.hpp"
#include "factory.hpp"
#include "detail/test_caller.hpp"
#include "../output.hpp"

namespace mettle {

  namespace detail {

    template<typename T>
    std::string annotate_type(const std::string &s) {
      return s + " (" + type_name<T>() + ")";
    }

    template<typename Tuple>
    struct compiled_subsuite_helper;

    template<typename ...T>
    struct compiled_subsuite_helper<std::tuple<T...>> {
      using type = compiled_suite<void(T&...)>;
    };

    template <typename ...T>
    std::array<std::common_type_t<T...>, sizeof...(T)>
    constexpr inline make_array(T &&...t) {
      return {{t...}};
    }

    template<template<typename ...> class T, typename ...Args>
    struct apply_type {
      template<typename ...Rest>
      using type = T<Args..., Rest...>;
    };


    template<template<typename ...> class Builder, typename ...Args,
             typename Factory, typename F>
    auto
    do_build(const std::string &name, const attributes &attrs,
             Factory &&factory, const F &f) {
      using FactoryValue = typename std::remove_reference<Factory>::type;
      Builder<FactoryValue, Args...> builder(
        name, attrs, std::forward<Factory>(factory)
      );
      f(builder);
      return builder.finalize();
    }

    template<template<typename ...> class Builder, typename ...Args, typename F>
    inline auto
    do_build(const std::string &name, const attributes &attrs, const F &f) {
      return do_build<Builder, Args...>(name, attrs, auto_factory, f);
    }

    template<template<typename ...> class Builder, typename ...Args,
             typename Factory, typename F>
    inline auto
    do_build(const std::string &name, Factory &&factory, const F &f) {
      return do_build<Builder, Args...>(
        name, {}, std::forward<Factory>(factory), f
      );
    }

    template<template<typename ...> class Builder, typename ...Args, typename F>
    inline auto
    do_build(const std::string &name, const F &f) {
      return do_build<Builder, Args...>(name, {}, auto_factory, f);
    }


    template<template<typename ...> class Builder, typename ...Args>
    inline auto
    do_builds(const std::string &name, const attributes &attrs,
              Args &&...args) {
      return make_array(do_build<Builder>(
        name, attrs, std::forward<Args>(args)...
      ));
    }

    template<template<typename ...> class Builder, typename Fixture,
             typename ...Args>
    inline auto
    do_builds(const std::string &name, const attributes &attrs,
              Args &&...args) {
      return make_array(do_build<Builder, Fixture>(
        name, attrs, std::forward<Args>(args)...
      ));
    }

    template<template<typename ...> class Builder, typename First,
             typename Second, typename ...Rest, typename ...Args>
    inline auto
    do_builds(const std::string &name, const attributes &attrs,
              Args &&...args) {
      using detail::annotate_type;
      return make_array(
        do_build<Builder, First> (annotate_type<First> (name), attrs, args...),
        do_build<Builder, Second>(annotate_type<Second>(name), attrs, args...),
        do_build<Builder, Rest>  (annotate_type<Rest>  (name), attrs, args...)
        ...
      );
    }

    template<template<typename ...> class Builder, typename ...Fixture,
             typename ...Args>
    inline auto
    do_builds(const std::string &name, Args &&...args) {
      return do_builds<Builder, Fixture...>(
        name, {}, std::forward<Args>(args)...
      );
    }

  } // namespace detail

  template<typename Tuple>
  using compiled_subsuite = typename detail::compiled_subsuite_helper<Tuple>
    ::type;

  template<typename ParentFixture, typename ...Fixture, typename ...Args>
  std::array<compiled_subsuite<ParentFixture>,
             std::max(sizeof...(Fixture), std::size_t(1))>
  make_subsuites(const std::string &name, const attributes &attrs,
                 Args &&...args);

  template<typename ParentFixture, typename ...Fixture, typename ...Args>
  std::array<compiled_subsuite<ParentFixture>,
             std::max(sizeof...(Fixture), std::size_t(1))>
  make_subsuites(const std::string &name, Args &&...args);

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

    void test(std::string name, attributes attrs, function_type f) {
      tests_.push_back({
        std::move(name), std::move(f), std::move(attrs)
      });
    }

    void subsuite(compiled_suite<void(T&...)> subsuite) {
      subsuites_.push_back(std::move(subsuite));
    }

    template<typename Subsuites>
    void subsuite(Subsuites &&subsuites) {
      for(auto &&i : subsuites)
        subsuites_.push_back(detail::forward_if<Subsuites>(i));
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
    std::vector<compiled_suite<void(T&...)>> subsuites_;
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

  template<typename ParentFixture, typename Factory, typename ...Fixture>
  class subsuite_builder
    : public suite_builder_base_t<Factory, ParentFixture, Fixture...> {
  private:
    static_assert(sizeof...(Fixture) < 2, "only specify one fixture at a time");
    using base = suite_builder_base_t<Factory, ParentFixture, Fixture...>;
  public:
    using factory_type = Factory;
    using fixture_type = detail::first_t<Fixture...>;
    using compiled_suite_type = compiled_subsuite<ParentFixture>;
    using function_type = typename base::function_type;

    subsuite_builder(const std::string &name, const attributes &attrs,
                     Factory factory)
      : base(name, attrs), factory_(factory) {}

    compiled_suite_type finalize() {
      return compiled_suite_type(
        std::move(base::name_), std::move(base::tests_),
        std::move(base::subsuites_), std::move(base::attrs_),
        [this](function_type test) { return wrap_test(test); }
      );
    }
  private:
    auto wrap_test(function_type test) {
      return detail::test_caller<factory_type, ParentFixture, Fixture...>{
        factory_, base::setup_, base::teardown_, std::move(test)
      };
    }

    factory_type factory_;
  };

  template<typename Exception, typename Factory, typename ...T>
  class suite_builder : public suite_builder_base_t<Factory, std::tuple<>,
                                                    T...> {
  private:
    static_assert(sizeof...(T) < 2, "only specify one fixture at a time");
    using base = suite_builder_base_t<Factory, std::tuple<>, T...>;
  public:
    using factory_type = Factory;
    using exception_type = Exception;
    using fixture_type = detail::first_t<T...>;
    using compiled_suite_type = runnable_suite;
    using function_type = typename base::function_type;

    suite_builder(const std::string &name, const attributes &attrs,
                  Factory factory)
      : base(name, attrs), factory_(factory) {}

    compiled_suite_type finalize() {
      return compiled_suite_type(
        std::move(base::name_), std::move(base::tests_),
        std::move(base::subsuites_), std::move(base::attrs_),
        [this](function_type test) { return wrap_test(test); }
      );
    }
  private:
    auto wrap_test(function_type test) {
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
        } catch(const exception_type &e) {
          message = e.what();
        } catch(const std::exception &e) {
          message = std::string("Uncaught exception: ") + to_printable(e);
        } catch(...) {
          message = "Unknown exception";
        }

        return { passed, message };
      };
    }

    factory_type factory_;
  };


  template<typename Exception, typename ...Fixture, typename ...Args>
  inline runnable_suite
  make_basic_suite(const std::string &name, const attributes &attrs,
                   Args &&...args) {
    using Applied = detail::apply_type<suite_builder, Exception>;
    return detail::do_build<Applied::template type, Fixture...>(
      name, attrs, std::forward<Args>(args)...
    );
  }

  template<typename Exception, typename ...Fixture, typename ...Args>
  inline runnable_suite
  make_basic_suite(const std::string &name, Args &&...args) {
    using Applied = detail::apply_type<suite_builder, Exception>;
    return detail::do_build<Applied::template type, Fixture...>(
      name, std::forward<Args>(args)...
    );
  }

  template<typename Exception, typename ...Fixture, typename ...Args>
  inline auto
  make_basic_suites(const std::string &name, const attributes &attrs,
                    Args &&...args) {
    using Applied = detail::apply_type<suite_builder, Exception>;
    return detail::do_builds<Applied::template type, Fixture...>(
      name, attrs, std::forward<Args>(args)...
    );
  }

  template<typename Exception, typename ...Fixture, typename ...Args>
  inline auto
  make_basic_suites(const std::string &name, Args &&...args) {
    using Applied = detail::apply_type<suite_builder, Exception>;
    return detail::do_builds<Applied::template type, Fixture...>(
      name, std::forward<Args>(args)...
    );
  }


  template<typename ParentFixture, typename ...Fixture, typename ...Args>
  inline compiled_subsuite<ParentFixture>
  make_subsuite(const std::string &name, const attributes &attrs,
                Args &&...args) {
    using Applied = detail::apply_type<subsuite_builder, ParentFixture>;
    return detail::do_build<Applied::template type, Fixture...>(
      name, attrs, std::forward<Args>(args)...
    );
  }

  template<typename ParentFixture, typename ...Fixture, typename ...Args>
  inline compiled_subsuite<ParentFixture>
  make_subsuite(const std::string &name, Args &&...args) {
    using Applied = detail::apply_type<subsuite_builder, ParentFixture>;
    return detail::do_build<Applied::template type, Fixture...>(
      name, std::forward<Args>(args)...
    );
  }

  template<typename ParentFixture, typename ...Fixture, typename ...Args>
  inline std::array<compiled_subsuite<ParentFixture>,
                    std::max(sizeof...(Fixture), std::size_t(1))>
  make_subsuites(const std::string &name, const attributes &attrs,
                 Args &&...args) {
    using Applied = detail::apply_type<subsuite_builder, ParentFixture>;
    return detail::do_builds<Applied::template type, Fixture...>(
      name, attrs, std::forward<Args>(args)...
    );
  }

  template<typename ParentFixture, typename ...Fixture, typename ...Args>
  inline std::array<compiled_subsuite<ParentFixture>,
                    std::max(sizeof...(Fixture), std::size_t(1))>
  make_subsuites(const std::string &name, Args &&...args) {
    using Applied = detail::apply_type<subsuite_builder, ParentFixture>;
    return detail::do_builds<Applied::template type, Fixture...>(
      name, std::forward<Args>(args)...
    );
  }


  template<typename ...Fixture, typename Parent, typename ...Args>
  inline auto
  make_subsuite(const Parent &, const std::string &name,
                const attributes &attrs, Args &&...args) {
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
  make_subsuites(const Parent &, const std::string &name,
                 const attributes &attrs, Args &&...args) {
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


  template<typename Parent, typename F>
  inline void
  setup(Parent &builder, F &&f) {
    builder.setup(std::forward<F>(f));
  }

  template<typename Parent, typename F>
  inline void
  teardown(Parent &builder, F &&f) {
    builder.teardown(std::forward<F>(f));
  }


  template<typename Parent, typename F>
  inline void
  test(Parent &builder, const std::string &name, const attributes &attrs,
       F &&f) {
    builder.test(name, attrs, std::forward<F>(f));
  }

  template<typename ...Fixture, typename Parent, typename F>
  inline void
  test(Parent &builder, const std::string &name, F &&f) {
    builder.test(name, std::forward<F>(f));
  }


  template<typename T>
  struct fixture_type {
    using type = typename std::remove_reference_t<T>::fixture_type;
  };

  template<typename T>
  using fixture_type_t = typename fixture_type<T>::type;

} // namespace mettle

#endif
