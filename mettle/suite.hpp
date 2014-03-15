#ifndef INC_METTLE_SUITE_HPP
#define INC_METTLE_SUITE_HPP

#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace mettle {

namespace detail {
  template<size_t ...>
  struct index_sequence { };

  template<size_t N, size_t ...S>
  struct make_index_seq_impl : make_index_seq_impl<N-1, N-1, S...> { };

  template<size_t ...S>
  struct make_index_seq_impl<0, S...> {
    typedef index_sequence<S...> type;
  };

  template<size_t I>
  using make_index_sequence = typename make_index_seq_impl<I>::type;

  template <typename F, typename Tuple, size_t... I>
  void apply_impl(F&& f, Tuple&& t, index_sequence<I...>) {
    return std::forward<F>(f)(std::get<I>(std::forward<Tuple>(t))...);
  }

  template<typename F, typename Tuple>
  void apply(F&& f, Tuple&& t) {
    using Indices = make_index_sequence<
      std::tuple_size<typename std::decay<Tuple>::type>::value
      >;
    return apply_impl(std::forward<F>(f), std::forward<Tuple>(t), Indices{});
  }
}

struct suite_results {
  suite_results(const std::string &name) : suite_name(name) {}

  size_t total_tests() const {
    return passes.size() + fails.size() + skips.size();
  }

  std::string suite_name;
  std::vector<std::string> passes;
  std::vector<std::pair<std::string, std::string>> fails;
  std::vector<std::string> skips;
};

std::vector<std::function<suite_results(void)>> suites;

template<typename ...T>
class suite {
public:
  using function_type = std::function<void(T&...)>;

  suite(const std::string &name, const std::function<void()> &f) : name_(name) {
    f();
    suites.push_back(std::bind(&suite::run, this));
  }

  void setup(const function_type &f) {
    setup_ = f;
  }

  void teardown(const function_type &f) {
    teardown_ = f;
  }

  void skip_test(const std::string &name, const function_type &f) {
    tests_.push_back({ name, f, true });
  }

  void test(const std::string &name, const function_type &f) {
    tests_.push_back({ name, f });
  }

  suite_results run() {
    suite_results results(name_);

    for(auto test : tests_) {
      std::tuple<T...> fixtures;
      if(setup_)
        detail::apply(setup_, fixtures);

      if(test.skip) {
        results.skips.push_back(test.name);
        continue;
      }

      try {
        detail::apply(test.function, fixtures);
        results.passes.push_back(test.name);
      }
      catch (const expectation_error &e) {
        results.fails.push_back({ test.name, e.what() });
      }
      catch(...) {
        results.fails.push_back({ test.name, "unknown error" });
      }

      if(teardown_)
        detail::apply(teardown_, fixtures);
    }

    return results;
  }
private:
  struct test_info {
    test_info(const std::string &name, const function_type &function,
              bool skip = false)
      : name(name), function(function), skip(skip) {}

    std::string name;
    function_type function;
    bool skip;
  };

  std::string name_;
  function_type setup_, teardown_;
  std::vector<test_info> tests_;
};

} // namespace mettle

#endif
