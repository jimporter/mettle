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
#include "../detail/algorithm.hpp"
#include "../detail/source_location.hpp"


namespace mettle {

  template<typename Factory, typename ParentFixture,
           typename Fixture = detail::no_fixture_t>
  class suite_builder;

  namespace detail {

    template<typename T>
    std::string annotate_type(const std::string &s) {
      return s + " (" + type_name<T>() + ")";
    }

    template<typename Exception>
    struct wrap_test {
      using compiled_suite_type = runnable_suite;

      template<typename T>
      auto operator ()(T &&t) const {
        return [test_function = std::move(t)]() mutable -> test_result {
          bool passed = false;
          std::string message;

          try {
            test_function();
            passed = true;
          } catch(const Exception &e) {
            message = e.what();
          } catch(const std::exception &e) {
            message = std::string("Uncaught exception: ") + to_printable(e);
          } catch(...) {
            message = "Unknown exception";
          }

          return { passed, message };
        };
      }
    };

    template<typename Wrap, typename Builder, typename = std::void_t<>>
    struct wrapped_suite {
      using type = compiled_suite<typename Builder::parent_fixture_signature>;
    };

    template<typename Wrap, typename Builder>
    struct wrapped_suite<Wrap, Builder, std::void_t<
      typename Wrap::compiled_suite_type
    >> {
      using type = typename Wrap::compiled_suite_type;
    };

    template<typename Builder, typename Wrap>
    typename wrapped_suite<Wrap, Builder>::type
    finalize(Builder &b, const Wrap &wrap) {
      return {
        std::move(b.name_), std::move(b.tests_), std::move(b.subsuites_),
        std::move(b.attrs_),
        [&b, &wrap](auto &&test) {
          return wrap(b.make_test_caller(std::move(test)));
        }
      };
    }

    template<typename ParentFixture, typename ...Fixture, typename Factory,
             typename F, typename Wrap>
    auto
    do_build(const std::string &name, const attributes &attrs,
             Factory &&factory, const F &f, const Wrap &wrap) {
      using factory_type = std::remove_reference_t<Factory>;
      suite_builder<factory_type, ParentFixture, Fixture...> builder(
        name, attrs, std::forward<Factory>(factory)
      );
      f(builder);
      return finalize(builder, wrap);
    }

    template<typename ParentFixture, typename ...Fixture, typename F,
             typename Wrap>
    inline auto
    do_build(const std::string &name, const attributes &attrs, const F &f,
             const Wrap &wrap) {
      return do_build<ParentFixture, Fixture...>(
        name, attrs, auto_factory, f, wrap
      );
    }

    template<typename ParentFixture, typename ...Fixture, typename ...Args>
    inline auto
    do_builds(const std::string &name, const attributes &attrs,
              Args &&...args) {
      if constexpr(sizeof...(Fixture) < 2) {
        return std::array{do_build<ParentFixture, Fixture...>(
          name, attrs, std::forward<Args>(args)...
        )};
      } else {
        return std::array{do_build<ParentFixture, Fixture>(
          detail::annotate_type<Fixture>(name), attrs, args...
        )...};
      }
    }

  } // namespace detail

  template<typename Exception, typename ...Fixture, typename ...Args>
  inline auto
  make_basic_suite(const std::string &name, const attributes &attrs,
                   Args &&...args) {
    return detail::do_build<std::tuple<>, Fixture...>(
      name, attrs, std::forward<Args>(args)..., detail::wrap_test<Exception>{}
    );
  }

  template<typename Exception, typename ...Fixture, typename ...Args>
  inline auto
  make_basic_suite(const std::string &name, Args &&...args) {
    return detail::do_build<std::tuple<>, Fixture...>(
      name, {}, std::forward<Args>(args)..., detail::wrap_test<Exception>{}
    );
  }

  template<typename Exception, typename ...Fixture, typename ...Args>
  inline auto
  make_basic_suites(const std::string &name, const attributes &attrs,
                    Args &&...args) {
    return detail::do_builds<std::tuple<>, Fixture...>(
      name, attrs, std::forward<Args>(args)..., detail::wrap_test<Exception>{}
    );
  }

  template<typename Exception, typename ...Fixture, typename ...Args>
  inline auto
  make_basic_suites(const std::string &name, Args &&...args) {
    return detail::do_builds<std::tuple<>, Fixture...>(
      name, {}, std::forward<Args>(args)..., detail::wrap_test<Exception>{}
    );
  }


  template<typename ParentFixture, typename ...Fixture, typename ...Args>
  inline auto
  make_subsuite(const std::string &name, const attributes &attrs,
                Args &&...args) {
    return detail::do_build<ParentFixture, Fixture...>(
      name, attrs, std::forward<Args>(args)..., std::identity{}
    );
  }

  template<typename ParentFixture, typename ...Fixture, typename ...Args>
  inline auto
  make_subsuite(const std::string &name, Args &&...args) {
    return detail::do_build<ParentFixture, Fixture...>(
      name, {}, std::forward<Args>(args)..., std::identity{}
    );
  }

  template<typename ParentFixture, typename ...Fixture, typename ...Args>
  inline auto
  make_subsuites(const std::string &name, const attributes &attrs,
                 Args &&...args) {
    return detail::do_builds<ParentFixture, Fixture...>(
      name, attrs, std::forward<Args>(args)..., std::identity{}
    );
  }

  template<typename ParentFixture, typename ...Fixture, typename ...Args>
  inline auto
  make_subsuites(const std::string &name, Args &&...args) {
    return detail::do_builds<ParentFixture, Fixture...>(
      name, {}, std::forward<Args>(args)..., std::identity{}
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

    void test(std::string name, function_type f,
                detail::source_location loc = detail::source_location::current()) {
      tests_.push_back({ std::move(name), std::move(f), {}, std::move(loc) });
    }

    void test(std::string name, attributes attrs, function_type f,
                detail::source_location loc = detail::source_location::current()) {
      tests_.push_back({ std::move(name), std::move(f), std::move(attrs), std::move(loc) });
    }

    void subsuite(compiled_suite<void(T&...)> subsuite) {
      subsuites_.push_back(std::move(subsuite));
    }

    template<typename Subsuites>
    void subsuite(Subsuites &&subsuites) {
      for(auto &&i : subsuites)
        subsuites_.push_back(detail::forward_like<Subsuites>(i));
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
      detail::source_location loc;
    };

    std::string name_;
    attributes attrs_;
    function_type setup_, teardown_;
    std::vector<test_info> tests_;
    std::vector<compiled_suite<void(T&...)>> subsuites_;
  };

  template<typename Factory, typename ...ParentFixture,
           typename Fixture /* = detail::no_fixture_t*/>
  requires(std::same_as<detail::transform_fixture_t<Factory, Fixture>, void>)
  class suite_builder<Factory, std::tuple<ParentFixture...>, Fixture>
    : public suite_builder_base<ParentFixture...> {
    using base = suite_builder_base<ParentFixture...>;
  public:
    using factory_type = Factory;
    using parent_fixture_type = std::tuple<ParentFixture...>;
    using parent_fixture_signature = void(ParentFixture&...);
    using fixture_type = Fixture;

    suite_builder(const std::string &name, const attributes &attrs,
                  Factory)
      : base(name, attrs) {}
  private:
    detail::test_caller<ParentFixture...>
    make_test_caller(typename base::function_type &&test) {
      return {base::setup_, base::teardown_, std::move(test)};
    }

    template<typename Builder, typename Wrap>
    friend typename detail::wrapped_suite<Wrap, Builder>::type
    detail::finalize(Builder &, const Wrap &);
  };

  template<typename Factory, typename ...ParentFixture,
           typename Fixture /* = detail::no_fixture_t*/>
  class suite_builder<Factory, std::tuple<ParentFixture...>, Fixture>
    : public suite_builder_base<ParentFixture...,
                                detail::transform_fixture_t<Factory, Fixture>> {
    using base = suite_builder_base<
      ParentFixture..., detail::transform_fixture_t<Factory, Fixture>
    >;
  public:
    using factory_type = Factory;
    using parent_fixture_type = std::tuple<ParentFixture...>;
    using parent_fixture_signature = void(ParentFixture&...);
    using fixture_type = Fixture;

    suite_builder(const std::string &name, const attributes &attrs,
                  Factory factory)
      : base(name, attrs), factory_(factory) {}
  private:
    detail::fixture_test_caller<Factory, Fixture, ParentFixture...>
    make_test_caller(typename base::function_type &&test) {
      return {{base::setup_, base::teardown_, std::move(test)}, factory_};
    }

    template<typename Builder, typename Wrap>
    friend typename detail::wrapped_suite<Wrap, Builder>::type
    detail::finalize(Builder &, const Wrap &);

    factory_type factory_;
  };


  template<typename ...Fixture, typename Parent, typename ...Args>
  inline void
  subsuite(Parent &p, const std::string &name, const attributes &attrs,
           Args &&...args) {
    p.template subsuite<Fixture...>(name, attrs, std::forward<Args>(args)...);
  }

  template<typename ...Fixture, typename Parent, typename ...Args>
  inline void
  subsuite(Parent &p, const std::string &name, Args &&...args) {
    p.template subsuite<Fixture...>(name, std::forward<Args>(args)...);
  }


  template<typename Parent, typename F>
  inline void setup(Parent &p, F &&f) {
    p.setup(std::forward<F>(f));
  }

  template<typename Parent, typename F>
  inline void teardown(Parent &p, F &&f) {
    p.teardown(std::forward<F>(f));
  }


  template<typename Parent, typename F>
  inline void
  test(Parent &p, const std::string &name, const attributes &attrs, F &&f) {
    p.test(name, attrs, std::forward<F>(f));
  }

  template<typename ...Fixture, typename Parent, typename F>
  inline void
  test(Parent &p, const std::string &name, F &&f) {
    p.test(name, std::forward<F>(f));
  }


  template<typename T>
  struct fixture_type {
    using type = typename std::remove_reference_t<T>::fixture_type;
  };

  template<typename T>
  using fixture_type_t = typename fixture_type<T>::type;

} // namespace mettle

#endif
