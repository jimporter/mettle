#ifndef INC_METTLE_DETAIL_SOURCE_LOCATION_HPP
#define INC_METTLE_DETAIL_SOURCE_LOCATION_HPP

#include <ciso646>

// Try to use `std::source_location`, except on Clang 15, where it's broken.
// See <https://github.com/llvm/llvm-project/issues/56379>.
#if __has_include(<source_location>) &&                 \
    (!defined(__clang__) || __clang_major__ >= 16)
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
#  include <experimental/source_location>
#  define METTLE_FOUND_SOURCE_LOCATION
namespace mettle::detail {
  using source_location = std::experimental::source_location;
}
#endif

#ifndef METTLE_FOUND_SOURCE_LOCATION
#  include "source_location_shim.hpp"
namespace mettle::detail {
  using source_location = source_location_shim;
}
#endif

#undef METTLE_FOUND_SOURCE_LOCATION

#endif
