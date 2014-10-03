#ifndef INC_METTLE_SUITE_GLOBAL_SUITE_HPP
#define INC_METTLE_SUITE_GLOBAL_SUITE_HPP

#include "detail/all_suites.hpp"
#include "make_suite.hpp"

namespace mettle {

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
    : basic_suite(detail::all_suites(), name, attrs,
                  std::forward<Args>(args)...) {}

  template<typename ...Args>
  basic_suite(const std::string &name, Args &&...args)
    : basic_suite(name, {}, std::forward<Args>(args)...) {}
};

} // namespace mettle

#endif
