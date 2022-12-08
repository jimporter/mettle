#ifndef INC_METTLE_DETAIL_SOURCE_LOCATION_HPP
#define INC_METTLE_DETAIL_SOURCE_LOCATION_HPP

#include <ciso646>

#if __cplusplus > 201703L && __has_include(<source_location>)
#  include <source_location>
#  ifdef __cpp_lib_source_location
#    define METTLE_FOUND_SOURCE_LOCATION
namespace mettle::detail {
  using source_location = std::source_location;
}
#  endif
#endif

#if !defined(METTLE_FOUND_SOURCE_LOCATION) &&           \
    __has_include(<experimental/source_location>)
// Older versions of clang (8 and below) don't have `__builtin_FILE` and so
// won't work with libstdc++'s `std:experimental::source_location`.
#  if defined(__clang__)
#    if __has_builtin(__builtin_FILE)
#      include <experimental/source_location>
#      define METTLE_FOUND_SOURCE_LOCATION
namespace mettle::detail {
  using source_location = std::experimental::source_location;
}
#    endif
#  else
#    include <experimental/source_location>
#    define METTLE_FOUND_SOURCE_LOCATION
namespace mettle::detail {
  using source_location = std::experimental::source_location;
}
#  endif
#endif

#ifndef METTLE_FOUND_SOURCE_LOCATION
#  include "source_location_shim.hpp"
namespace mettle::detail {
  using source_location = source_location_shim;
}
#endif

#undef METTLE_FOUND_SOURCE_LOCATION

#endif
