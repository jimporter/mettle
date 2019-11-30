#ifndef INC_METTLE_SUITE_DETAIL_ALL_SUITES_HPP
#define INC_METTLE_SUITE_DETAIL_ALL_SUITES_HPP

#include "../compiled_suite.hpp"

namespace mettle::detail {

  inline suites_list & all_suites() {
    static suites_list all;
    return all;
  }

} // namespace mettle::detail

#endif
