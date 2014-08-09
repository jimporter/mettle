#ifndef INC_METTLE_COMPILED_SUITE_HPP
#define INC_METTLE_COMPILED_SUITE_HPP

#include <atomic>
#include <functional>
#include <string>
#include <vector>

namespace mettle {

namespace detail {
  static inline size_t generate_id() {
    static std::atomic<size_t> id_(0);
    return id_++;
  }
}

struct test_result {
  bool passed;
  std::string message;
};

template<typename Ret, typename ...T>
class compiled_suite {
public:
  struct test_info {
    using function_type = std::function<Ret(T&...)>;

    test_info(const std::string &name, const function_type &function,
              bool skip = false)
      : name(name), function(function), skip(skip),
        id(detail::generate_id()) {}

    std::string name;
    function_type function;
    bool skip;
    size_t id;
  };

  using iterator = typename std::vector<test_info>::const_iterator;

  template<typename U, typename V, typename Func>
  compiled_suite(const std::string &name, const U &tests, const V &subsuites,
                 const Func &f) : name_(name) {
    for(const auto &test : tests)
      tests_.push_back(f(test));
    for(const auto &ss : subsuites)
      subsuites_.push_back(compiled_suite(ss, f));
  }

  template<typename Ret2, typename ...T2, typename Func>
  compiled_suite(const compiled_suite<Ret2, T2...> &suite, const Func &f)
    : compiled_suite(suite.name(), suite, suite.subsuites(), f) {}

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
using test_function = std::function<test_result(void)>;

} // namespace mettle

#endif
