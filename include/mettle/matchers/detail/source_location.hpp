#ifndef INC_METTLE_MATCHERS_DETAIL_SOURCE_LOCATION_HPP
#define INC_METTLE_MATCHERS_DETAIL_SOURCE_LOCATION_HPP

// Get _LIBCPP_VERSION to detect libc++.
#include <ciso646>

#if defined(METTLE_USE_STDLIB_EXTS)
#  include <experimental/source_location>
#  define METTLE_SOURCE_LOCATION std::experimental::source_location
#else
   // XXX: clang probably won't support libstdc++'s source_location, since it
   // lacks the required compiler magic. Just disable it for now. See
   // <https://gcc.gnu.org/ml/gcc-patches/2015-11/msg01687.html>.
#  if !(defined(__clang__) && !defined(_LIBCPP_VERSION) && \
        defined(__GLIBCXX__)) && __has_include(<experimental/source_location>)
#    include <experimental/source_location>
#    define METTLE_SOURCE_LOCATION std::experimental::source_location
#  else
#    include "source_location_shim.hpp"
#    define METTLE_SOURCE_LOCATION mettle::detail::source_location
#  endif
#endif

#endif
