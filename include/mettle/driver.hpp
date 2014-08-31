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
  template<typename ...Args>
  basic_suite(suites_list &list, const std::string &name,
              const attributes &attrs, Args &&...args) {
    auto &&suites = make_basic_suites<Exception, Fixture...>(
      name, attrs, std::forward<Args>(args)...
    );
    for(auto &&i : suites)
      list.push_back(std::move(i));
  }

  template<typename ...Args>
  basic_suite(suites_list &list, const std::string &name, Args &&...args)
    : basic_suite(list, name, {}, std::forward<Args>(args)...) {}

  template<typename ...Args>
  basic_suite(const std::string &name, const attributes &attrs, Args &&...args)
    : basic_suite(detail::all_suites, name, attrs,
                  std::forward<Args>(args)...) {}

  template<typename ...Args>
  basic_suite(const std::string &name, Args &&...args)
    : basic_suite(name, {}, std::forward<Args>(args)...) {}
};

template<typename ...T>
using suite = basic_suite<expectation_error, T...>;

} // namespace mettle

int main(int argc, const char *argv[]) {
  return mettle::detail::real_main(argc, argv);
}

#endif
