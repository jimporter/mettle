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

class runnable_suite {
public:
  struct test_result {
    bool passed;
    std::string message;
  };

  struct test_info {
    using function_type = std::function<test_result()>;

    test_info(const std::string &name, const function_type &function,
              bool skip = false)
      : name(name), function(function), skip(skip) {}

    std::string name;
    function_type function;
    bool skip;
  };

  using iterator = std::vector<test_info>::const_iterator;

  template<typename T, typename Func>
  runnable_suite(const std::string &name, const T &begin, const T &end,
                 const Func &f) : name_(name) {
    for(auto i = begin; i != end; ++i) {
      tests_.push_back(f(*i));
    }
  }

  iterator begin() const {
    return tests_.begin();
  }

  iterator end() const {
    return tests_.end();
  }

  const std::string & name() const {
    return name_;
  }

  size_t size() const {
    return tests_.size();
  }
private:
  std::string name_;
  std::vector<test_info> tests_;
};

template<typename Exception, typename ...T>
class suite_builder {
public:
  using exception_type = Exception;
  using function_type = std::function<void(T&...)>;

  suite_builder(const std::string &name) : name_(name) {}
  suite_builder(const suite_builder &) = delete;
  suite_builder & operator =(const suite_builder &) = delete;

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

  std::shared_ptr<runnable_suite> finalize() {
    using namespace std::placeholders;
    return std::make_shared<runnable_suite>(
      name_, tests_.begin(), tests_.end(),
      std::bind(&suite_builder::make_test, this, _1)
    );
  }
private:
  struct test_info {
    std::string name;
    function_type f;
    bool skip;
  };

  runnable_suite::test_info make_test(const test_info &test) {
    auto &f = test.f;
    auto &setup = setup_;
    auto &teardown = teardown_;

    runnable_suite::test_info::function_type test_function = [
      f, setup, teardown
    ]() -> runnable_suite::test_result {
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
      catch (const exception_type &e) {
        message = e.what();
      }
      catch(...) {
        message = "unknown error";
      }

      return { passed, message };
    };

    return { test.name, test_function, test.skip };
  }

  std::string name_;
  function_type setup_, teardown_;
  std::vector<test_info> tests_;
};

template<typename Exception, typename ...T, typename F>
auto make_basic_suite(const std::string &name, const F &f) {
  suite_builder<Exception, T...> builder(name);
  f(builder);
  return builder.finalize();
}

} // namespace mettle

#endif
