#ifndef INC_METTLE_DETAIL_ALL_SUITES_HPP
#define INC_METTLE_DETAIL_ALL_SUITES_HPP

#include <vector>

#include "../compiled_suite.hpp"

namespace mettle {

using suites_list = std::vector<runnable_suite>;

namespace detail {
  inline suites_list & all_suites() {
    static suites_list all;
    return all;
  }
}

} // namespace mettle

#endif
