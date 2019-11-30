#ifndef INC_METTLE_DRIVER_LIB_DRIVER_HPP
#define INC_METTLE_DRIVER_LIB_DRIVER_HPP

#include "detail/export.hpp"
#include "../suite/detail/all_suites.hpp"
#include "../suite/detail/built_in_attrs.hpp"

namespace mettle::detail {

  METTLE_PUBLIC int
  drive_tests(int argc, const char *argv[], const suites_list &suites);

} // namespace mettle::detail

int main(int argc, const char *argv[]) {
  return mettle::detail::drive_tests(argc, argv, mettle::detail::all_suites());
}

#endif
