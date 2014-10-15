#ifndef INC_METTLE_DETAIL_LIB_DRIVER_HPP
#define INC_METTLE_DETAIL_LIB_DRIVER_HPP

#include "../../suite/detail/all_suites.hpp"

namespace mettle {

namespace detail {
  int drive_tests(int argc, const char *argv[], const suites_list &suites);
}

} // namespace mettle

int main(int argc, const char *argv[]) {
  return mettle::detail::drive_tests(argc, argv, mettle::detail::all_suites());
}

#endif
