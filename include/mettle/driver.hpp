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
  template<typename F>
  basic_suite(const std::string &name, const F &f,
              suites_list &suites = detail::all_suites) {
    for(auto &&i : make_basic_suites<Exception, Fixture...>(name, f))
      suites.push_back(std::move(i));
  }
};

template<typename Exception, typename ...Fixture>
struct skip_basic_suite {
  template<typename F>
  skip_basic_suite(const std::string &name, const F &f,
                   suites_list &suites = detail::all_suites) {
    for(auto &&i : make_skip_basic_suites<Exception, Fixture...>(name, f))
      suites.push_back(std::move(i));
  }
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
