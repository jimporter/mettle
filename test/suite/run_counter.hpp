#ifndef INC_METTLE_TEST_SUITE_RUN_COUNTER_HPP
#define INC_METTLE_TEST_SUITE_RUN_COUNTER_HPP

#include <cstdint>
#include <functional>
#include <memory>

template<typename ...T>
class run_counter {
public:
  run_counter(const std::function<void(T...)> &f = nullptr)
    : f_(f), runs_(std::make_shared<std::size_t>(0)) {}

  void operator ()(T &...t) {
    (*runs_)++;
    if(f_)
      f_(t...);
  }

  std::size_t runs() const {
    return *runs_;
  }
private:
  std::function<void(T...)> f_;
  std::shared_ptr<std::size_t> runs_;
};

#endif
