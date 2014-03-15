#ifndef INC_METTLE_SUITE_HPP
#define INC_METTLE_SUITE_HPP

#include <functional>
#include <iostream>
#include <string>
#include <utility>
#include <vector>

namespace mettle {

std::vector<std::function<void()>> suites;

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

  void test(const std::string &name, const function_type &f) {
    tests_.push_back({ name, f });
  }

  void run() {
    std::vector<std::string> passes;
    std::vector<std::pair<std::string, std::string>> fails;

    for(auto test : tests_) {
      std::tuple<T...> fixtures;
      if(setup_)
        detail::apply(setup_, fixtures);

      try {
        detail::apply(test.second, fixtures);
        passes.push_back(test.first);
      }
      catch (const expectation_error &e) {
        fails.push_back({ test.first, e.what() });
      }
      catch(...) {
        fails.push_back({ test.first, "unknown error" });
      }

      if(teardown_)
        detail::apply(teardown_, fixtures);
    }

    std::cout << passes.size() << "/" << (passes.size() + fails.size())
              << " tests passed" << std::endl;
    if (fails.size()) {
      for (auto i : fails) {
        std::cout << " " << i.first << " FAILED: " << i.second << std::endl;
      }
    }
  }
private:
  std::string name_;
  function_type setup_, teardown_;
  std::vector<std::pair<std::string, function_type>> tests_;
};

} // namespace mettle

#endif
