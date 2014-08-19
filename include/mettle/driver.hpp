#ifndef INC_METTLE_DRIVER_HPP
#define INC_METTLE_DRIVER_HPP

#include <vector>

#include "glue.hpp"

namespace mettle {

using suites_list = std::vector<runnable_suite>;

namespace detail {
  extern suites_list all_suites;

  int real_main(int argc, const char *argv[]);
}

template<typename Exception, typename ...Fixture>
struct basic_suite {
  template<typename Factory, typename F>
  basic_suite(suites_list &list, const std::string &name, Factory &&factory,
              F &&f) {
    auto &&suites = make_basic_suites<Exception, Fixture...>(
      name, std::forward<Factory>(factory), std::forward<F>(f)
    );
    for(auto &&i : suites)
      list.push_back(std::move(i));
  }

  template<typename F>
  basic_suite(suites_list &list, const std::string &name, F &&f)
    : basic_suite(list, name, auto_factory, std::forward<F>(f)) {}

  template<typename Factory, typename F>
  basic_suite(const std::string &name, Factory &&factory, F &&f)
    : basic_suite(detail::all_suites, name, std::forward<Factory>(factory),
                  std::forward<F>(f)) {}

  template<typename F>
  basic_suite(const std::string &name, const F &f)
    : basic_suite(detail::all_suites, name, auto_factory, f) {}
};

template<typename Exception, typename ...Fixture>
struct skip_basic_suite {
  template<typename Factory, typename F>
  skip_basic_suite(suites_list &list, const std::string &name,
                   Factory &&factory, F &&f) {
    auto &&suites = make_skip_basic_suites<Exception, Fixture...>(
      name, std::forward<Factory>(factory), std::forward<F>(f)
    );
    for(auto &&i : suites)
      list.push_back(std::move(i));
  }

  template<typename F>
  skip_basic_suite(suites_list &list, const std::string &name, F &&f)
    : skip_basic_suite(list, name, auto_factory, std::forward<F>(f)) {}

  template<typename Factory, typename F>
  skip_basic_suite(const std::string &name, Factory &&factory, F &&f)
    : skip_basic_suite(detail::all_suites, name, std::forward<Factory>(factory),
                       std::forward<F>(f)) {}

  template<typename F>
  skip_basic_suite(const std::string &name, const F &f)
    : skip_basic_suite(detail::all_suites, name, auto_factory, f) {}
};

template<typename ...T>
using suite = basic_suite<expectation_error, T...>;

template<typename ...T>
using skip_suite = skip_basic_suite<expectation_error, T...>;

} // namespace mettle

int main(int argc, const char *argv[]) {
  return mettle::detail::real_main(argc, argv);
}

#endif
