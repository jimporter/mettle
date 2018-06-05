#ifndef INC_METTLE_DRIVER_DETAIL_OPTIONAL_HPP
#define INC_METTLE_DRIVER_DETAIL_OPTIONAL_HPP

// Try to use std::optional, N4480's optional class, or fall back to Boost's.

#ifdef __has_include
#  if __has_include(<optional>) && (__cplusplus >= 201703L || \
      (defined(_MSVC_LANG) && _MSVC_LANG >= 201703L))
#    include <optional>
#    define METTLE_OPTIONAL_NS std
#  elif __has_include(<experimental/optional>) && \
        !defined(METTLE_NO_STDLIB_EXTS)
#    include <experimental/optional>
#    define METTLE_OPTIONAL_NS std::experimental
#  endif
#endif

#ifndef METTLE_OPTIONAL_NS
#  ifdef METTLE_USE_STDLIB_EXTS
#    include <experimental/optional>
#    define METTLE_OPTIONAL_NS std::experimental
#  else
#    include <boost/optional.hpp>
#    include <boost/optional/optional_io.hpp>
#    define METTLE_OPTIONAL_NS boost
#    define METTLE_OPTIONAL_USING_BOOST
#  endif
#endif

#endif
