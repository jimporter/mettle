#ifndef INC_METTLE_MATCHERS_DETAIL_SOURCE_LOCATION_HPP
#define INC_METTLE_MATCHERS_DETAIL_SOURCE_LOCATION_HPP

// Older versions of clang (8 and below) don't have `__builtin_FILE` and so
// won't work with `std[::experimental]::source_location`.
#if defined(__clang__)
#  if !__has_builtin(__builtin_FILE)
#    include "source_location_shim.hpp"
#    define METTLE_SOURCE_LOCATION mettle::detail::source_location
#  endif
#endif

#ifndef METTLE_SOURCE_LOCATION
#  if __has_include(<source_location>)
#    include <source_location>
#    define METTLE_SOURCE_LOCATION std::source_location
#  elif __has_include(<experimental/source_location>)
#    include <experimental/source_location>
#    define METTLE_SOURCE_LOCATION std::experimental::source_location
#  else
#    include "source_location_shim.hpp"
#    define METTLE_SOURCE_LOCATION mettle::detail::source_location
#  endif
#endif

#endif
