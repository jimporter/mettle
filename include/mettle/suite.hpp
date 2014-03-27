#ifndef INC_METTLE_SUITE_HPP
#define INC_METTLE_SUITE_HPP

#include <algorithm>
#include <functional>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace mettle {

namespace detail {
  template<size_t ...>
  struct index_sequence {};

  template<size_t N, size_t ...S>
  struct make_index_seq_impl : make_index_seq_impl<N-1, N-1, S...> { };

  template<size_t ...S>
  struct make_index_seq_impl<0, S...> {
    typedef index_sequence<S...> type;
  };

  template<size_t I>
  using make_index_sequence = typename make_index_seq_impl<I>::type;

  template <typename F, typename Tuple, size_t... I>
  auto apply_impl(F &&f, Tuple &&t, index_sequence<I...>) {
    return std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))...);
  }

  template<typename F, typename Tuple>
  auto apply(F &&f, Tuple &&t) {
    using Indices = make_index_sequence<
      std::tuple_size<typename std::decay<Tuple>::type>::value
    >;
    return apply_impl(std::forward<F>(f), std::forward<Tuple>(t), Indices());
  }
}

struct test_result {
  bool passed;
  std::string message;
};

template<typename Ret, typename ...T>
struct compiled_suite {
  struct test_info {
    using function_type = std::function<Ret(T&...)>;

    test_info(const std::string &name, const function_type &function,
              bool skip = false)
      : name(name), function(function), skip(skip) {}

    std::string name;
    function_type function;
    bool skip;
  };

  using iterator = typename std::vector<test_info>::const_iterator;

  template<typename U, typename V, typename Func>
  compiled_suite(const std::string &name, const U &tests, const V &subsuites,
                 const Func &f) : name_(name) {
    for(auto &test : tests)
      tests_.push_back(f(test));
    for(auto &ss : subsuites)
      subsuites_.push_back(compiled_suite(ss, f));
  }

  template<typename Ret2, typename ...T2, typename Func>
  compiled_suite(const compiled_suite<Ret2, T2...> &suite, const Func &f)
    : name_(suite.name()) {
    for(auto &test : suite)
      tests_.push_back(f(test));
    for(auto &ss : suite.subsuites())
      subsuites_.push_back(compiled_suite(ss, f));
  }

  const std::string & name() const {
    return name_;
  }

  iterator begin() const {
    return tests_.begin();
  }

  iterator end() const {
    return tests_.end();
  }

  size_t size() const {
    return tests_.size();
  }

  const std::vector<compiled_suite> & subsuites() const {
    return subsuites_;
  }
private:
  std::string name_;
  std::vector<test_info> tests_;
  std::vector<compiled_suite> subsuites_;
};

using runnable_suite = compiled_suite<test_result>;

template<typename Parent, typename ...T>
class subsuite_builder;

template<typename ...T>
class suite_builder_base {
public:
  using raw_function_type = void(T&...);
  using function_type = std::function<raw_function_type>;
  using tuple_type = std::tuple<T...>;

  suite_builder_base(const std::string &name) : name_(name) {}
  suite_builder_base(const suite_builder_base &) = delete;
  suite_builder_base & operator =(const suite_builder_base &) = delete;

  void setup(const function_type &f) {
    setup_ = f;
  }

  void teardown(const function_type &f) {
    teardown_ = f;
  }

  void skip_test(const std::string &name, const function_type &f) {
    tests_.push_back({name, f, true});
  }

  void test(const std::string &name, const function_type &f) {
    tests_.push_back({name, f, false});
  }

  void subsuite(const compiled_suite<void, T...> &subsuite) {
    subsuites_.push_back(subsuite);
  }

  void subsuite(compiled_suite<void, T...> &&subsuite) {
    subsuites_.push_back(std::move(subsuite));
  }

  template<typename ...U, typename F>
  void subsuite(const std::string &name, const F &f) {
    subsuite_builder<tuple_type, U...> builder(name);
    f(builder);
    subsuite(builder.finalize());
  }
protected:
  struct test_info {
    std::string name;
    function_type function;
    bool skip;
  };

  std::string name_;
  function_type setup_, teardown_;
  std::vector<test_info> tests_;
  std::vector<compiled_suite<void, T...>> subsuites_;
};

template<typename ...T, typename ...U>
class subsuite_builder<std::tuple<T...>, U...>
  : public suite_builder_base<T..., U...> {
private:
  using compiled_suite_type = compiled_suite<void, T...>;
  using base = suite_builder_base<T..., U...>;
public:
  using base::base;

  compiled_suite_type finalize() const {
    return compiled_suite_type(
      base::name_, base::tests_, base::subsuites_,
      [this](const auto &a) { return wrap_test(a); }
    );
  }
private:
  template<typename V>
  typename compiled_suite_type::test_info wrap_test(const V &test) const {
    auto &f = test.function;
    auto &setup = base::setup_;
    auto &teardown = base::teardown_;

    typename compiled_suite_type::test_info::function_type test_function = [
      f, setup, teardown
    ](T &...args) -> void {
      std::tuple<T&..., U...> fixtures(args..., U()...);
      if(setup)
        detail::apply(setup, fixtures);
      detail::apply(f, fixtures);
      if(teardown)
        detail::apply(teardown, fixtures);
    };

    return { test.name, test_function, test.skip };
  }
};

template<typename Exception, typename ...T>
class suite_builder : public suite_builder_base<T...> {
private:
  using base = suite_builder_base<T...>;
public:
  using exception_type = Exception;
  using base::base;

  runnable_suite finalize() const {
    return runnable_suite(
      base::name_, base::tests_, base::subsuites_,
      [this](const auto &a) { return wrap_test(a); }
    );
  }
private:
  template<typename U>
  runnable_suite::test_info wrap_test(const U &test) const {
    auto &f = test.function;
    auto &setup = base::setup_;
    auto &teardown = base::teardown_;

    runnable_suite::test_info::function_type test_function = [
      f, setup, teardown
    ]() -> test_result {
      bool passed = false;
      std::string message;

      try {
        std::tuple<T...> fixtures;
        if(setup)
          detail::apply(setup, fixtures);
        detail::apply(f, fixtures);
        if(teardown)
          detail::apply(teardown, fixtures);
        passed = true;
      }
      catch(const exception_type &e) {
        message = e.what();
      }
      catch(...) {
        message = "unknown error";
      }

      return { passed, message };
    };

    return { test.name, test_function, test.skip };
  }
};

template<typename Exception, typename ...T, typename F>
runnable_suite make_basic_suite(const std::string &name, const F &f) {
  suite_builder<Exception, T...> builder(name);
  f(builder);
  return builder.finalize();
}

template<typename T, typename ...U, typename F>
auto make_subsuite(const std::string &name, const F &f) {
  subsuite_builder<T, U...> builder(name);
  f(builder);
  return builder.finalize();
}

template<typename ...T, typename Parent, typename F>
auto make_subsuite(const Parent &, const std::string &name, const F &f) {
  return make_subsuite<typename Parent::tuple_type, T...>(name, f);
}

template<typename ...T, typename Parent, typename F>
void subsuite(Parent &builder, const std::string &name, const F &f) {
  builder.subsuite(make_subsuite<T...>(builder, name, f));
}

} // namespace mettle

#endif
