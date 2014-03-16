#ifndef INC_METTLE_SUITE_HPP
#define INC_METTLE_SUITE_HPP

#include <algorithm>
#include <functional>
#include <iostream>
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

class suite_base;
std::vector<suite_base*> suites;

class suite_base {
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

  suite_base(const std::string &name, const std::function<void()> &f)
    : name_(name) {
    f();
    suites.push_back(this);
  }

  virtual ~suite_base() {
    suites.erase(std::find(suites.begin(), suites.end(), this));
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
protected:
  std::string name_;
  std::vector<test_info> tests_;
};

template<typename ...T>
class suite : public suite_base {
public:
  using function_type = std::function<void(T&...)>;

  using suite_base::suite_base;

  void setup(const function_type &f) {
    setup_ = f;
  }

  void teardown(const function_type &f) {
    teardown_ = f;
  }

  void skip_test(const std::string &name, const function_type &f) {
    add_test(name, f, true);
  }

  void test(const std::string &name, const function_type &f) {
    add_test(name, f, false);
  }
private:
  void add_test(const std::string &name, const function_type &f, bool skip) {
    test_info::function_type test_function = [f, this]() -> test_result {
      std::tuple<T...> fixtures;
      if(setup_)
        detail::apply(setup_, fixtures);

      bool passed = false;
      std::string message;
      try {
        detail::apply(f, fixtures);
        passed = true;
      }
      catch (const expectation_error &e) {
        message = e.what();
      }
      catch(...) {
        message = "unknown error";
      }

      if(teardown_)
        detail::apply(teardown_, fixtures);

      return { passed, message };
    };

    tests_.push_back({ name, test_function, skip });
  }

  function_type setup_, teardown_;
};

} // namespace mettle

#endif
